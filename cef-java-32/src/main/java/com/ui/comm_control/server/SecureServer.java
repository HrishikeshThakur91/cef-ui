package com.anca.appl.fw.gui.comm_control.server;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpsConfigurator;
import com.sun.net.httpserver.HttpsServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import javax.crypto.Cipher;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import java.awt.Desktop;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.KeyStore;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Simple embedded HTTPS server that serves static files from {@code resources/certs/dist}.
 *
 * <p>Also includes a temporary control-file watcher that simulates CEF commands.
 * This is a stopgap until CEF/gRPC is restored.
 *
 * <p><b>Control file format:</b>
 * <ul>
 *   <li>Encrypted with AES-256-GCM using the shared key at {@code tmp/secret.key}</li>
 *   <li>Decrypted content is a JSON object with {@code type} and optional {@code payload} fields</li>
 *   <li>Supported types: {@code START}, {@code NAVIGATE}, {@code SHUTDOWN}</li>
 * </ul>
 *
 * <p><b>Note:</b> This server is a temporary bridge and is not intended for production use.
 */
public class SecureServer {

    // -------------------------------------------------------------------------
    // Constants
    // -------------------------------------------------------------------------

    private static final Logger logger = LogManager.getLogger(SecureServer.class);

    private static final String USER_DIR          = System.getProperty("user.dir");
    private static final String CONTROL_FILE      = Path.of(USER_DIR, "tmp", "control.dat").toAbsolutePath().toString();
    private static final String SHARED_KEY_PATH   = Path.of(USER_DIR, "tmp", "secret.key").toAbsolutePath().toString();

    private static final String KEYSTORE_PATH     = "resources/certs/server.jks";
    private static final char[] KEYSTORE_PASSWORD = "changeit".toCharArray();
    private static final String DIST_BASE_PATH    = "resources/certs/dist";

    private static final int    SERVER_PORT       = 8443;
    private static final String LOCALHOST_BASE    = "https://localhost:" + SERVER_PORT;

    private static final String CMD_START         = "START";
    private static final String CMD_NAVIGATE      = "NAVIGATE";
    private static final String CMD_SHUTDOWN      = "SHUTDOWN";
    private static final String NAVIGATE_PREFIX   = "NAVIGATE:";
    private static final String START_PREFIX      = "START";

    private static final int    GCM_IV_LENGTH     = 12;
    private static final int    GCM_TAG_BITS      = 128;
    private static final int    GCM_MIN_LENGTH    = GCM_IV_LENGTH + 16; // IV + auth tag

    private static final long   WATCHER_POLL_MS   = 500L;
    private static final int    STOP_DELAY_SEC    = 3;
    private static final long   WATCHER_JOIN_MS   = 2_000L;
    private static final long   EXECUTOR_TERM_SEC = 5L;

    /**
     * MIME type lookup table keyed by lowercase file extension (including the leading dot).
     * Populated in a static initialiser to remain compatible with Java 11.
     */
    private static final Map<String, String> MIME_TYPES;

    static {
        Map<String, String> map = new HashMap<>();
        map.put(".html", "text/html");
        map.put(".js",   "application/javascript");
        map.put(".css",  "text/css");
        map.put(".json", "application/json");
        map.put(".png",  "image/png");
        map.put(".jpg",  "image/jpeg");
        map.put(".jpeg", "image/jpeg");
        map.put(".svg",  "image/svg+xml");
        map.put(".ico",  "image/x-icon");
        MIME_TYPES = Collections.unmodifiableMap(map);
    }

    // -------------------------------------------------------------------------
    // Mutable server state
    // (volatile fields or AtomicBoolean; mutations happen inside synchronized methods)
    // -------------------------------------------------------------------------

    private static volatile HttpsServer     server;
    private static volatile ExecutorService executor;
    private static volatile Thread          controlWatcherThread;
    private static volatile String          currentlyOpenUrl;

    private static final AtomicBoolean running = new AtomicBoolean(false);

    // -------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------

    /**
     * Private constructor – {@code SecureServer} is a utility class and must not
     * be instantiated directly.
     */
    private SecureServer() {
        throw new UnsupportedOperationException("SecureServer is a utility class");
    }

    // -------------------------------------------------------------------------
    // Lifecycle – start
    // -------------------------------------------------------------------------

    /**
     * Starts the HTTPS server and the control-file watcher background thread.
     *
     * <p>If the server is already running this method logs a message and returns
     * immediately without performing any action.
     *
     * @param args command-line arguments (currently unused)
     * @throws FileNotFoundException if the JKS keystore file does not exist at
     *                               {@value #KEYSTORE_PATH}
     * @throws Exception             if the keystore cannot be loaded or the server
     *                               socket cannot be bound
     */
    public static synchronized void start(String[] args) throws Exception {
        if (running.get()) {
            logger.info("[Manual Server] Already running – ignoring start request");
            return;
        }

        SSLContext sslContext = buildSslContext();

        server = HttpsServer.create(new InetSocketAddress(SERVER_PORT), 0);
        server.setHttpsConfigurator(new HttpsConfigurator(sslContext));

        executor = Executors.newFixedThreadPool(Math.max(4, Runtime.getRuntime().availableProcessors()));
        server.setExecutor(executor);

        server.createContext("/", SecureServer::handleHttpRequest);
        server.start();

        running.set(true);
        logger.info("[Manual Server] HTTPS server running at {}", LOCALHOST_BASE);

        startControlFileWatcher();
    }

    // -------------------------------------------------------------------------
    // Lifecycle – stop
    // -------------------------------------------------------------------------

    /**
     * Gracefully stops the HTTPS server, the control-file watcher thread, and the
     * thread-pool executor in that order.
     *
     * <p>If the server is not currently running this method returns immediately.
     */
    public static synchronized void stop() {
        if (!running.get()) {
            return;
        }

        logger.info("[Manual Server] Stopping...");
        running.set(false);

        stopControlWatcher();
        stopHttpServer();
        stopExecutor();
        deleteControlFile();

        logger.info("[Manual Server] Stopped cleanly");
    }

    /**
     * Deletes the encrypted control file from disk during server shutdown.
     *
     * <p>This prevents stale commands from being replayed if the server is
     * restarted before a new control file is written by the caller.
     *
     * <p>Deletion is attempted on a best-effort basis; failure is logged as a
     * warning rather than thrown, since it should not block a clean shutdown.
     */
    private static void deleteControlFile()
    {
        File controlFile = new File(CONTROL_FILE);

        if (!controlFile.exists())
        {
            logger.debug("[Manual Server] Control file not present – nothing to delete");
            return;
        }

        if (controlFile.delete())
        {
            logger.info("[Manual Server] Control file deleted: {}", CONTROL_FILE);
        }
        else
        {
            logger.warn("[Manual Server] Failed to delete control file: {}", CONTROL_FILE);
        }
    }

    // -------------------------------------------------------------------------
    // HTTP request handler
    // -------------------------------------------------------------------------

    /**
     * Handles every incoming HTTPS request by serving a static file from
     * {@value #DIST_BASE_PATH}.
     *
     * <p>Routing rules applied in order:
     * <ol>
     *   <li>The root path {@code /} is rewritten to {@code /index.html}.</li>
     *   <li>Requests for {@code .dat} files are rejected with HTTP {@code 404} to
     *       prevent accidental exposure of control or key files.</li>
     *   <li>Paths that do not resolve to an existing regular file fall back to
     *       {@code index.html} to support SPA client-side routing.</li>
     *   <li>All other files are served with the correct MIME type and HTTP {@code 200}.</li>
     * </ol>
     *
     * @param exchange the HTTP exchange provided by the embedded {@link HttpsServer};
     *                 must not be {@code null}
     * @throws IOException if a low-level I/O error prevents writing the response headers
     */
    private static void handleHttpRequest(HttpExchange exchange) throws IOException {
        try {
            String uriPath = exchange.getRequestURI().getPath();

            if (uriPath == null || uriPath.equals("/") || uriPath.isEmpty()) {
                uriPath = "/index.html";
            }

            // Block .dat file downloads to protect control/key files
            if (uriPath.toLowerCase().endsWith(".dat")) {
                exchange.sendResponseHeaders(404, -1);
                return;
            }

            File distBase = new File(DIST_BASE_PATH);
            File file     = new File(distBase, uriPath.substring(1));

            // SPA fallback: unmapped paths serve index.html
            if (!file.exists() || file.isDirectory()) {
                file = new File(distBase, "index.html");
            }

            byte[] bytes = Files.readAllBytes(file.toPath());
            exchange.getResponseHeaders().set("Content-Type", getMimeType(file.getName()));
            exchange.sendResponseHeaders(200, bytes.length);

            try {
                exchange.getResponseBody().write(bytes);
                exchange.getResponseBody().flush();
            } catch (IOException clientAbort) {
                // Client closed the connection before the response completed – normal on localhost
                logger.debug("[Manual Server] Client aborted connection for {}", uriPath);
            }

        } catch (Exception e) {
            logger.error("[Manual Server] Error serving request: {}", e.getMessage(), e);
            exchange.sendResponseHeaders(500, -1);
        } finally {
            exchange.close();
        }
    }

    // -------------------------------------------------------------------------
    // Control-file watcher
    // -------------------------------------------------------------------------

    /**
     * Starts a background daemon thread named {@code "control-file-watcher"} that
     * polls the control file every {@value #WATCHER_POLL_MS} ms and dispatches
     * commands whenever the file's last-modified timestamp advances.
     *
     * <p><b>TEMPORARY:</b> This mechanism replaces CEF integration and will be
     * removed once CEF/gRPC is restored.
     */
    private static void startControlFileWatcher() {
        controlWatcherThread = new Thread(SecureServer::watchControlFile, "control-file-watcher");
        controlWatcherThread.setDaemon(true);
        controlWatcherThread.start();
    }

    /**
     * Body of the control-file watcher loop, executed on the
     * {@code "control-file-watcher"} daemon thread.
     *
     * <p>On each poll iteration the method:
     * <ol>
     *   <li>Sleeps for {@value #WATCHER_POLL_MS} ms.</li>
     *   <li>Skips if the control file does not exist or has not been modified.</li>
     *   <li>Reads and decrypts the file contents using AES-GCM.</li>
     *   <li>Attempts to parse the plaintext as a JSON command object.</li>
     *   <li>Silently discards duplicate commands sharing the same {@code commandId}.</li>
     *   <li>Delegates valid JSON commands to {@link #dispatchCommand(JSONObject)}.</li>
     *   <li>Falls back to {@link #processLegacyCommand(String)} for non-JSON content.</li>
     * </ol>
     *
     * <p>The loop exits cleanly when {@link #running} becomes {@code false} or the
     * thread is interrupted.
     */
    private static void watchControlFile() {
        File   controlFile   = new File(CONTROL_FILE);
        long   lastModified  = 0L;
        String lastCommandId = "";

        while (running.get() && !Thread.currentThread().isInterrupted()) {
            try {
                Thread.sleep(WATCHER_POLL_MS);

                if (!controlFile.exists()) {
                    continue;
                }

                long modified = controlFile.lastModified();
                if (modified <= lastModified) {
                    continue;
                }
                lastModified = modified;

                byte[] encryptedBytes = Files.readAllBytes(controlFile.toPath());
                byte[] secretKey      = Files.readAllBytes(Path.of(SHARED_KEY_PATH));
                byte[] decryptedBytes = decrypt(encryptedBytes, secretKey);

                String raw = new String(decryptedBytes, StandardCharsets.UTF_8).trim();

                JSONObject cmd = parseJson(raw);
                if (cmd == null) {
                    logger.warn("[Manual Server] Decrypted data is not valid JSON – falling back to legacy handling");
                    if (!raw.isEmpty()) {
                        processLegacyCommand(raw);
                    }
                    continue;
                }

                String cmdId = objectToString(cmd.get("commandId"), "");
                if (cmdId.equals(lastCommandId)) {
                    continue; // duplicate – ignore
                }
                lastCommandId = cmdId;

                dispatchCommand(cmd);

            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            } catch (Exception e) {
                logger.error("[Manual Server] Control watcher error: {}", e.getMessage(), e);
            }
        }
    }

    /**
     * Dispatches a parsed JSON command object to the appropriate handler method.
     *
     * <p>Recognised {@code type} values (case-insensitive):
     * <ul>
     *   <li>{@code START}    – opens the server root in the system default browser.</li>
     *   <li>{@code NAVIGATE} – navigates the browser to the URL in {@code payload.url}.</li>
     *   <li>{@code SHUTDOWN} – stops the server.</li>
     * </ul>
     *
     * <p>Commands with an unrecognised or missing {@code type} are logged and ignored.
     *
     * @param cmd the parsed JSON command object; must not be {@code null}
     */
    private static void dispatchCommand(JSONObject cmd) {
        String type       = objectToString(cmd.get("type"), null);
        Object payloadObj = cmd.get("payload");

        if (type == null) {
            logger.warn("[Manual Server] Command has no 'type' field – ignoring");
            return;
        }

        String upperType = type.toUpperCase();

        if (CMD_START.equals(upperType)) {
            String rawUrl = extractUrlStringFromPayload(payloadObj);
            String url    = extractUrlFromPayload(rawUrl);
            openBrowser(url != null ? url : LOCALHOST_BASE + "/");

        } else if (CMD_NAVIGATE.equals(upperType)) {
            String rawUrl = extractUrlStringFromPayload(payloadObj);
            if (rawUrl == null || rawUrl.trim().isEmpty()) {
                logger.warn("[Manual Server] NAVIGATE command is missing payload.url");
                return;
            }
            String url = extractUrlFromPayload(rawUrl.trim());
            if (url != null) {
                openBrowser(url);
            }

        } else if (CMD_SHUTDOWN.equals(upperType)) {
            logger.info("[Manual Server] SHUTDOWN command received");
            stop();

        } else {
            logger.warn("[Manual Server] Unhandled command type: {}", type);
        }
    }

    /**
     * Extracts the raw URL string from a command {@code payload} object.
     *
     * <p>If the payload is a {@link JSONObject} its {@code url} key is returned.
     * Otherwise the payload itself is converted to a string.
     *
     * @param payloadObj the {@code payload} value from the JSON command; may be {@code null}
     * @return the URL string, or {@code null} if the payload is {@code null} or does not
     *         contain a {@code url} entry
     */
    private static String extractUrlStringFromPayload(Object payloadObj) {
        if (payloadObj instanceof JSONObject) {
            JSONObject payloadJson = (JSONObject) payloadObj;
            return objectToString(payloadJson.get("url"), null);
        }
        return objectToString(payloadObj, null);
    }

    // -------------------------------------------------------------------------
    // Legacy (plain-text prefix) command handling
    // -------------------------------------------------------------------------

    /**
     * Handles raw (non-JSON) control commands for backwards compatibility with
     * legacy plain-text command writers.
     *
     * <p>Supported formats:
     * <ul>
     *   <li>{@code START[payload]} – opens the browser at the URL extracted from
     *       {@code payload}, or at the server root if extraction fails.</li>
     *   <li>{@code NAVIGATE:payload} – navigates to the URL extracted from
     *       {@code payload}.</li>
     *   <li>{@code SHUTDOWN} – stops the server.</li>
     * </ul>
     *
     * @param command the raw decrypted command string; must not be {@code null} or empty
     */
    private static void processLegacyCommand(String command) {
        logger.info("[Manual Server] Processing legacy command: {}", command);

        if (command.startsWith(START_PREFIX)) {
            String payload = command.substring(START_PREFIX.length()).trim();
            String url     = extractUrlFromPayload(payload);
            openBrowser(url != null ? url : LOCALHOST_BASE + "/");

        } else if (command.startsWith(NAVIGATE_PREFIX)) {
            String payload = command.substring(NAVIGATE_PREFIX.length()).trim();
            String url     = extractUrlFromPayload(payload);
            if (url != null) {
                openBrowser(url);
            } else {
                logger.warn("[Manual Server] NAVIGATE – could not extract URL from: {}", payload);
            }

        } else if (CMD_SHUTDOWN.equals(command)) {
            logger.info("[Manual Server] SHUTDOWN (legacy)");
            stop();

        } else {
            logger.warn("[Manual Server] Unknown legacy command: {}", command);
        }
    }

    // -------------------------------------------------------------------------
    // URL extraction
    // -------------------------------------------------------------------------

    /**
     * Extracts a fully-qualified URL from a raw payload string.
     *
     * <p>Two payload formats are supported:
     * <ol>
     *   <li><b>JSON object</b> – e.g. {@code {"docUrl":"https://host/page"}} – the
     *       value of {@code docUrl} is returned as-is.</li>
     *   <li><b>Simple path</b> – e.g. {@code /docs/intro} – {@link #LOCALHOST_BASE}
     *       is prepended to form an absolute URL.</li>
     * </ol>
     *
     * @param payload the raw payload string; may be {@code null} or blank
     * @return the fully-qualified URL string, or {@code null} if extraction fails
     */
    private static String extractUrlFromPayload(String payload) {
        if (payload == null || payload.trim().isEmpty()) {
            logger.error("[Manual Server] extractUrlFromPayload: payload is null or empty");
            return null;
        }

        if (payload.startsWith("{")) {
            JSONObject json = parseJson(payload);
            if (json == null) {
                logger.error("[Manual Server] extractUrlFromPayload: failed to parse JSON payload");
                return null;
            }
            Object docUrl = json.get("docUrl");
            if (docUrl == null) {
                logger.error("[Manual Server] extractUrlFromPayload: 'docUrl' not found. Keys: {}", json.keySet());
                return null;
            }
            return docUrl.toString().trim();
        }

        // Simple path → absolute localhost URL
        String url = LOCALHOST_BASE + payload;
        logger.debug("[Manual Server] extractUrlFromPayload: simple path → {}", url);
        return url;
    }

    // -------------------------------------------------------------------------
    // Browser
    // -------------------------------------------------------------------------

    /**
     * Opens the given URL in the system default browser via the JDK {@link Desktop} API.
     *
     * <p>If the URL does not already begin with {@link #LOCALHOST_BASE} it is
     * prefixed with it, ensuring all navigation remains within the local server.
     *
     * <p><b>Note:</b> the {@link Desktop} API does not provide control over browser tab
     * reuse; each call may open in a new tab depending on the OS and browser configuration.
     *
     * @param url the URL to open; if {@code null} the call is logged and silently ignored
     */
    private static void openBrowser(String url) {
        if (url == null) {
            logger.error("[Manual Server] openBrowser: received null URL – ignoring");
            return;
        }

        if (!url.startsWith(LOCALHOST_BASE)) {
            url = LOCALHOST_BASE + url;
        }

        if (!Desktop.isDesktopSupported()) {
            logger.warn("[Manual Server] Desktop API not supported on this platform – cannot open browser");
            return;
        }

        try {
            logger.info("[Manual Server] Opening browser → {}", url);
            currentlyOpenUrl = url;
            Desktop.getDesktop().browse(new URI(url));
        } catch (Exception e) {
            logger.error("[Manual Server] Failed to open browser for {}: {}", url, e.getMessage(), e);
        }
    }

    // -------------------------------------------------------------------------
    // Cryptography
    // -------------------------------------------------------------------------

    /**
     * Decrypts a byte array that was encrypted with {@code AES/GCM/NoPadding}.
     *
     * <p>Expected wire format:
     * <pre>{@code [12-byte GCM IV] [ciphertext bytes + 16-byte GCM auth tag]}</pre>
     *
     * <p>The JCE AES/GCM implementation appends the auth tag to the ciphertext
     * automatically; callers do not need to separate it.
     *
     * @param ciphertextWithIv the concatenated IV and ciphertext (including auth tag);
     *                         must not be {@code null} and must be at least
     *                         {@code GCM_IV_LENGTH + 16} = 28 bytes
     * @param key              raw AES key bytes; must be exactly 16, 24, or 32 bytes
     * @return the decrypted plaintext byte array
     * @throws IllegalArgumentException if {@code ciphertextWithIv} is too short or
     *                                  {@code key} length is not 16, 24, or 32
     * @throws Exception                if decryption fails, e.g. due to a GCM
     *                                  authentication tag mismatch
     */
    private static byte[] decrypt(byte[] ciphertextWithIv, byte[] key) throws Exception {
        if (ciphertextWithIv == null || ciphertextWithIv.length < GCM_MIN_LENGTH) {
            throw new IllegalArgumentException(
                    "Ciphertext too short – expected at least " + GCM_MIN_LENGTH + " bytes");
        }
        if (key == null || (key.length != 16 && key.length != 24 && key.length != 32)) {
            throw new IllegalArgumentException(
                    "Invalid AES key length: " + (key == null ? 0 : key.length));
        }

        byte[] iv         = new byte[GCM_IV_LENGTH];
        int    cipherLen  = ciphertextWithIv.length - GCM_IV_LENGTH;
        byte[] cipherText = new byte[cipherLen];

        System.arraycopy(ciphertextWithIv, 0,             iv,         0, GCM_IV_LENGTH);
        System.arraycopy(ciphertextWithIv, GCM_IV_LENGTH, cipherText, 0, cipherLen);

        Cipher           cipher  = Cipher.getInstance("AES/GCM/NoPadding");
        SecretKeySpec    keySpec = new SecretKeySpec(key, "AES");
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_BITS, iv);

        cipher.init(Cipher.DECRYPT_MODE, keySpec, gcmSpec);
        return cipher.doFinal(cipherText);
    }

    // -------------------------------------------------------------------------
    // SSL / TLS
    // -------------------------------------------------------------------------

    /**
     * Loads the JKS keystore from {@value #KEYSTORE_PATH} and constructs a
     * TLS {@link SSLContext} initialised with the server's key material.
     *
     * @return a fully initialised {@link SSLContext} ready for use with TLS
     * @throws FileNotFoundException if the keystore file does not exist at the
     *                               path {@value #KEYSTORE_PATH}
     * @throws Exception             if the keystore cannot be loaded or the SSL
     *                               context cannot be initialised
     */
    private static SSLContext buildSslContext() throws Exception {
        File keystoreFile = new File(KEYSTORE_PATH);
        if (!keystoreFile.exists()) {
            throw new FileNotFoundException("Keystore not found: " + keystoreFile.getAbsolutePath());
        }

        KeyStore ks = KeyStore.getInstance("JKS");
        try (InputStream is = new FileInputStream(keystoreFile)) {
            ks.load(is, KEYSTORE_PASSWORD);
        }

        KeyManagerFactory kmf = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        kmf.init(ks, KEYSTORE_PASSWORD);

        SSLContext sc = SSLContext.getInstance("TLS");
        sc.init(kmf.getKeyManagers(), null, null);
        return sc;
    }

    // -------------------------------------------------------------------------
    // MIME type resolution
    // -------------------------------------------------------------------------

    /**
     * Returns the MIME content-type for the given file name based on its extension.
     *
     * <p>Lookup is performed against {@link #MIME_TYPES} using the lower-cased
     * extension (including the leading dot). Returns {@code application/octet-stream}
     * for any extension not present in the map.
     *
     * <p>Note: {@code .dat} files are intentionally absent from the MIME table because
     * such requests are rejected at the handler level before this method is reached.
     *
     * @param fileName the bare file name (not a full path); must not be {@code null}
     * @return the MIME type string; never {@code null}
     */
    private static String getMimeType(String fileName) {
        int dotIndex = fileName.lastIndexOf('.');
        if (dotIndex < 0) {
            return "application/octet-stream";
        }
        String ext = fileName.substring(dotIndex).toLowerCase();
        return MIME_TYPES.getOrDefault(ext, "application/octet-stream");
    }

    // -------------------------------------------------------------------------
    // Lifecycle helpers
    // -------------------------------------------------------------------------

    /**
     * Interrupts the control-file watcher thread and waits up to
     * {@value #WATCHER_JOIN_MS} ms for it to terminate, then clears the field.
     * Has no effect if the watcher thread reference is {@code null}.
     */
    private static void stopControlWatcher() {
        if (controlWatcherThread == null) {
            return;
        }
        controlWatcherThread.interrupt();
        try {
            controlWatcherThread.join(WATCHER_JOIN_MS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        controlWatcherThread = null;
    }

    /**
     * Stops the HTTPS server, allowing up to {@value #STOP_DELAY_SEC} seconds for
     * in-flight requests to complete before the socket is closed, then clears the field.
     * Has no effect if the server reference is {@code null}.
     */
    private static void stopHttpServer() {
        if (server == null) {
            return;
        }
        server.stop(STOP_DELAY_SEC);
        server = null;
    }

    /**
     * Initiates an orderly shutdown of the thread-pool executor and waits up to
     * {@value #EXECUTOR_TERM_SEC} seconds for running tasks to finish.
     * Forces an immediate shutdown ({@link ExecutorService#shutdownNow()}) if the
     * timeout elapses, then clears the field.
     * Has no effect if the executor reference is {@code null}.
     */
    private static void stopExecutor() {
        if (executor == null) {
            return;
        }
        executor.shutdown();
        try {
            if (!executor.awaitTermination(EXECUTOR_TERM_SEC, TimeUnit.SECONDS)) {
                executor.shutdownNow();
            }
        } catch (InterruptedException e) {
            executor.shutdownNow();
            Thread.currentThread().interrupt();
        }
        executor = null;
    }

    // -------------------------------------------------------------------------
    // Utility helpers
    // -------------------------------------------------------------------------

    /**
     * Safely parses a JSON string and returns the root value as a {@link JSONObject}.
     *
     * @param raw the JSON text to parse; must not be {@code null}
     * @return the parsed {@link JSONObject}, or {@code null} if parsing fails or the
     *         root element is not a JSON object (e.g. it is a JSON array or primitive)
     */
    private static JSONObject parseJson(String raw) {
        try {
            Object parsed = new JSONParser().parse(raw);
            if (parsed instanceof JSONObject) {
                return (JSONObject) parsed;
            }
            return null;
        } catch (ParseException e) {
            logger.debug("[Manual Server] parseJson failed for input: {}", e.getMessage());
            return null;
        }
    }

    /**
     * Converts an object to its {@link Object#toString()} representation, returning a
     * caller-supplied default value when the object is {@code null}.
     *
     * @param obj          the object to convert; may be {@code null}
     * @param defaultValue the fallback value returned when {@code obj} is {@code null}
     * @return {@code obj.toString()} if {@code obj} is non-null, otherwise {@code defaultValue}
     */
    private static String objectToString(Object obj, String defaultValue) {
        return obj != null ? obj.toString() : defaultValue;
    }
}