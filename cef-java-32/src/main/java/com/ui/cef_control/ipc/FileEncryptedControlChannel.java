package com.ui.cef_control.ipc;

import org.json.simple.JSONObject;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import com.ui.cef_control.util.Logger;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;

/**
 * File-based encrypted control channel implementation.
 * 
 * Writes encrypted control commands to a file using AES-256-GCM encryption.
 * Uses atomic file replacement to ensure consistency.
 * 
 * File format: [12-byte IV][ciphertext][16-byte auth tag]
 * 
 * Thread Safety:
 * - Single-threaded only
 * - No concurrent access allowed
 * - Shutdown flag is not volatile (single-threaded access)
 * 
 * Constraints:
 * - AES-256-GCM only
 * - No IPC (file-based)
 * - No sockets
 * - No threads
 * - Single writer only
 * - Atomic file replace
 * - Uses Logger for logging
 * - No retries
 * - No health logic
 * - No process logic
 */
public final class FileEncryptedControlChannel implements IControlCommandChannel {

    private static final String CIPHER_ALGORITHM = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12; // 12 bytes for GCM
    private static final int GCM_TAG_LENGTH = 16; // 16 bytes (128 bits) for auth tag
    private static final int AES_KEY_LENGTH = 32; // 32 bytes for AES-256

    private final Path controlFile;
    private final SecretKey secretKey;
    private final SecureRandom secureRandom;
    private boolean shutdown; // Single-threaded access only

    /**
     * Creates a new FileEncryptedControlChannel.
     * 
     * @param controlFile the target file path for writing encrypted commands
     * @param base64Key   the AES-256 key encoded in Base64 (must decode to 32
     *                    bytes)
     * @throws NullPointerException     if controlFile or base64Key is null
     * @throws IllegalArgumentException if base64Key is invalid or not 32 bytes
     * @throws RuntimeException         if encryption initialization fails
     */
    public FileEncryptedControlChannel(Path controlFile, String base64Key) {
        if (controlFile == null) {
            throw new NullPointerException("controlFile cannot be null");
        }
        if (base64Key == null) {
            throw new NullPointerException("base64Key cannot be null");
        }

        this.controlFile = controlFile;
        this.secureRandom = new SecureRandom();
        this.shutdown = false;

        // Decode and validate the Base64 key
        byte[] keyBytes;
        try {
            keyBytes = Base64.getDecoder().decode(base64Key);
        } catch (IllegalArgumentException e) {
            String errorMsg = "Invalid Base64 key: " + e.getMessage();
            Logger.error("FileEncryptedControlChannel", errorMsg);
            throw new IllegalArgumentException(errorMsg, e);
        }

        if (keyBytes.length != AES_KEY_LENGTH) {
            String errorMsg = "Invalid key length: expected " + AES_KEY_LENGTH + " bytes, got " + keyBytes.length;
            Logger.error("FileEncryptedControlChannel", errorMsg);
            throw new IllegalArgumentException(errorMsg);
        }

        this.secretKey = new SecretKeySpec(keyBytes, "AES");

        Logger.info("FileEncryptedControlChannel", "Initialized with file: " + controlFile);
    }

    @Override
    public void sendCommand(ControlCommand command) {
        if (command == null) {
            throw new NullPointerException("command cannot be null");
        }

        if (shutdown) {
            String errorMsg = "Cannot send command: channel is shutdown";
            Logger.error("FileEncryptedControlChannel", errorMsg);
            throw new IllegalStateException(errorMsg);
        }

        try {
            // Serialize command to JSON
            String jsonString = serializeToJson(command);
            Logger.info("FileEncryptedControlChannel", "Serialized command: " + command.getCommandId());

            // Encrypt the JSON
            byte[] encryptedData = encrypt(jsonString);
            Logger.info("FileEncryptedControlChannel", "Encrypted command: " + encryptedData.length + " bytes");

            // Write atomically to file
            writeAtomically(encryptedData);
            Logger.info("FileEncryptedControlChannel", "Command written to file: " + controlFile);

        } catch (Exception e) {
            String errorMsg = "Failed to send command: " + e.getMessage();
            Logger.error("FileEncryptedControlChannel", errorMsg);
            throw new RuntimeException(errorMsg, e);
        }
    }

    @Override
    public void shutdown() {
        shutdown = true;
        Logger.info("FileEncryptedControlChannel", "Channel shutdown");
    }

    /**
     * Serializes a ControlCommand to JSON string.
     * 
     * @param command the command to serialize
     * @return JSON string representation
     */
    @SuppressWarnings("unchecked")
    private String serializeToJson(ControlCommand command) {
        JSONObject json = new JSONObject();
        json.put("commandId", command.getCommandId());
        json.put("type", command.getType().name());

        // Add payload if present
        Map<String, String> payload = command.getPayload();
        if (payload != null && !payload.isEmpty()) {
            JSONObject payloadJson = new JSONObject();
            payloadJson.putAll(payload);
            json.put("payload", payloadJson);
        } else {
            json.put("payload", null);
        }

        json.put("timestamp", command.getTimestamp());

        return json.toJSONString();
    }

    /**
     * Encrypts data using AES-256-GCM.
     * 
     * Format: [12-byte IV][ciphertext][16-byte auth tag]
     * 
     * @param plaintext the plaintext to encrypt
     * @return encrypted data with IV and auth tag
     * @throws Exception if encryption fails
     */
    private byte[] encrypt(String plaintext) throws Exception {
        // Generate random IV
        byte[] iv = new byte[GCM_IV_LENGTH];
        secureRandom.nextBytes(iv);

        // Initialize cipher
        Cipher cipher = Cipher.getInstance(CIPHER_ALGORITHM);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv); // tag length in bits
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmSpec);

        // Encrypt
        byte[] ciphertextWithTag = cipher.doFinal(plaintext.getBytes(StandardCharsets.UTF_8));

        // Combine: IV + ciphertext + tag
        // Note: cipher.doFinal() already appends the auth tag to the ciphertext
        ByteBuffer buffer = ByteBuffer.allocate(GCM_IV_LENGTH + ciphertextWithTag.length);
        buffer.put(iv);
        buffer.put(ciphertextWithTag);

        return buffer.array();
    }

    /**
     * Writes data to file atomically using temp file + rename.
     * 
     * @param data the data to write
     * @throws IOException if file operations fail
     */
    private void writeAtomically(byte[] data) throws IOException {
        // Create parent directory if it doesn't exist
        Path parentDir = controlFile.getParent();
        if (parentDir != null && !Files.exists(parentDir)) {
            Files.createDirectories(parentDir);
        }

        // Create temp file in the same directory
        Path tempFile = controlFile.resolveSibling(controlFile.getFileName() + ".tmp");

        try {
            // Write to temp file
            Files.write(tempFile, data);

            // Atomically replace target file
            Files.move(tempFile, controlFile,
                    StandardCopyOption.REPLACE_EXISTING,
                    StandardCopyOption.ATOMIC_MOVE);

        } catch (IOException e) {
            // Clean up temp file on error
            try {
                Files.deleteIfExists(tempFile);
            } catch (IOException cleanupError) {
                Logger.error("FileEncryptedControlChannel", "Failed to delete temp file: " + cleanupError.getMessage());
            }
            throw e;
        }
    }
}
