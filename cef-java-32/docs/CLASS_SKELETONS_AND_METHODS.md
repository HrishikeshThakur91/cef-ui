# Phase-6 Section 1: Class Skeletons & Key Methods

## Class: HttpServerConfig

**Package**: `com.ui.cef_control.http`

**Purpose**: Immutable configuration object for HTTP server

```java
public final class HttpServerConfig {
    private final String staticFilesPath;
    private final int port;
    private final String bindAddress;
    
    public HttpServerConfig(String staticFilesPath, int port, String bindAddress)
    public String getStaticFilesPath()
    public int getPort()
    public String getBindAddress()
}
```

**Key Methods**:
- Constructor validates all parameters (non-empty paths, valid port range 0-65535, non-empty bind address)
- Getters return immutable values

---

## Class: HttpServerListener (Interface)

**Package**: `com.ui.cef_control.http`

**Purpose**: Contract for lifecycle event notifications

```java
public interface HttpServerListener {
    void onBeforeRestart()
    void onAfterRestart(String newAddress)
    void onStartFailure(Throwable error)
}
```

**Key Methods**:
- `onBeforeRestart()`: Called before server restart begins
- `onAfterRestart(String)`: Called after restart with address "127.0.0.1:PORT"
- `onStartFailure(Throwable)`: Called when startup fails (for diagnostics)

---

## Class: VuePressHttpServer

**Package**: `com.ui.cef_control.http`

**Purpose**: Core HTTP server implementation

```java
public class VuePressHttpServer {
    private final HttpServerConfig config
    private final List<HttpServerListener> listeners
    private com.sun.net.httpserver.HttpServer httpServer
    private boolean running
    
    public VuePressHttpServer(HttpServerConfig config)
    public void start() throws IOException
    public void stop()
    public String getActualAddress()
    public int getActualPort()
    public boolean isRunning()
    public void addListener(HttpServerListener listener)
    public void removeListener(HttpServerListener listener)
    
    // Private methods
    private void notifyBeforeRestart()
    private void notifyAfterRestart(String newAddress)
    private void notifyStartFailure(Throwable error)
}
```

**Key Methods**:

### `start() throws IOException`
```
Logic:
1. Validate static files path exists (throw IOException if not)
2. Create HttpServer bound to bindAddress:port from config
   - If port == 0, OS assigns ephemeral port
3. Create StaticFileHandler(staticPath)
4. Register handler on "/" context
5. Call httpServer.start()
6. Set running = true
7. Get actual bound address from httpServer.getAddress()
8. Call notifyAfterRestart(actualAddress)
```

### `stop()`
```
Logic:
1. Check running == true (throw if false)
2. Call httpServer.stop(0) [graceful, indefinite wait]
3. Set httpServer = null
4. Set running = false
```

### `getActualAddress()` → String
```
Logic:
1. If not running or httpServer == null, return null
2. Get InetSocketAddress from httpServer.getAddress()
3. Return "hostname:port" format
```

### `getActualPort()` → int
```
Logic:
1. If not running or httpServer == null, return -1
2. Get InetSocketAddress from httpServer.getAddress()
3. Return port number
```

---

## Class: StaticFileHandler

**Package**: `com.ui.cef_control.http`

**Purpose**: HTTP request handler for static file serving

```java
class StaticFileHandler implements HttpHandler {
    private final Path staticRoot
    
    StaticFileHandler(Path staticRoot)
    
    @Override
    public void handle(HttpExchange exchange) throws IOException
    
    // Private methods
    private void serveFile(HttpExchange exchange, Path filePath) throws IOException
    private String guessContentType(Path filePath)
}
```

**Key Methods**:

### `handle(HttpExchange exchange)`
```
Logic:
1. Check request method == "GET"
   - If not: sendResponseHeaders(405, 0) and close
2. Extract path from exchange.getRequestURI()
3. Remove leading "/" from path
4. Check path contains ".." (directory traversal)
   - If yes: sendResponseHeaders(400, 0) and close
5. Resolve requested path: filePath = staticRoot.resolve(path)
6. If Files.isRegularFile(filePath):
      - Call serveFile(exchange, filePath)
   Else:
      - Try index.html: indexPath = staticRoot.resolve("index.html")
      - If Files.isRegularFile(indexPath):
            - Call serveFile(exchange, indexPath)
        Else:
            - sendResponseHeaders(404, 0) and close
```

### `serveFile(HttpExchange exchange, Path filePath)`
```
Logic:
1. Read file content: byte[] content = Files.readAllBytes(filePath)
2. Determine content-type: guessContentType(filePath)
3. Set header: exchange.getResponseHeaders().set("Content-Type", contentType)
4. Send headers: exchange.sendResponseHeaders(200, content.length)
5. Write content to response body
6. Close response body
```

### `guessContentType(Path filePath)` → String
```
Logic:
1. Extract file extension from filePath.getFileName()
2. Return mime type based on extension:
   - .html → "text/html; charset=utf-8"
   - .css → "text/css"
   - .js → "application/javascript"
   - .json → "application/json"
   - .svg → "image/svg+xml"
   - .png → "image/png"
   - .jpg/.jpeg → "image/jpeg"
   - .gif → "image/gif"
   - .webp → "image/webp"
   - .woff → "font/woff"
   - .woff2 → "font/woff2"
   - .ttf → "font/ttf"
   - .eot → "application/vnd.ms-fontobject"
   - default → Files.probeContentType(filePath) or "application/octet-stream"
```

---

## Class: HttpServerSupervisor

**Package**: `com.ui.cef_control.http`

**Purpose**: Supervisor for HTTP server lifecycle with retry management

```java
public class HttpServerSupervisor {
    private final VuePressHttpServer server
    private final RetryPolicy retryPolicy
    private final List<HttpServerListener> listeners
    private boolean serverRunning
    
    public HttpServerSupervisor(VuePressHttpServer server, RetryPolicy retryPolicy)
    public void start()
    public void stop()
    public boolean isServerRunning()
    public String getServerAddress()
    public int getServerPort()
    public void addListener(HttpServerListener listener)
    public void removeListener(HttpServerListener listener)
    
    // Private methods
    private void notifyBeforeRestart()
    private void notifyAfterRestart(String newAddress)
    private void notifyStartFailure(Throwable error)
}
```

**Key Methods**:

### `start()`
```
Logic:
int attempt = 1
Loop:
    Try:
        1. Call notifyBeforeRestart()
        2. Call server.start()
        3. Set serverRunning = true
        4. Get actualAddress = server.getActualAddress()
        5. Call notifyAfterRestart(actualAddress)
        6. Return [success]
    
    Catch IOException e:
        1. Call notifyStartFailure(e)
        2. If not retryPolicy.shouldRetry(attempt, e):
              - Throw RuntimeException("Failed after N attempts", e)
        3. Get backoffMs = retryPolicy.getBackoffMs(attempt)
        4. Thread.sleep(backoffMs)
        5. attempt++
        6. Continue loop
```

### `stop()`
```
Logic:
1. Check serverRunning == true (throw IllegalStateException if false)
2. Call server.stop()
3. Set serverRunning = false
```

### `getServerAddress()` → String
```
Logic:
Return server.getActualAddress()
```

### `getServerPort()` → int
```
Logic:
Return server.getActualPort()
```

---

## Interface Extension: RetryPolicy (Phase-6 Update)

**Package**: `com.ui.cef_control.supervisor`

**Added Method**:

```java
public interface RetryPolicy {
    // Existing method
    boolean shouldRetry(int attempt, Throwable failure)
    
    // New Phase-6 method (default implementation)
    default long getBackoffMs(int attempt) {
        // Exponential backoff: 100ms * 2^(attempt-1)
        return 100L * (1L << (attempt - 1));
    }
}
```

**Exponential Backoff Examples**:
- Attempt 1: 100ms * 2^0 = 100ms
- Attempt 2: 100ms * 2^1 = 200ms
- Attempt 3: 100ms * 2^2 = 400ms
- Attempt 4: 100ms * 2^3 = 800ms
- Attempt 5: 100ms * 2^4 = 1600ms

---

## Lifecycle State Flow

```
HttpServerSupervisor.start()
    ↓
    notifyBeforeRestart() ──→ HttpServerListener.onBeforeRestart()
    ↓
    server.start()
        ├─ [Success]
        │   ├─ httpServer created
        │   ├─ StaticFileHandler registered
        │   ├─ server.start() called
        │   ├─ serverRunning = true
        │   └─ notifyAfterRestart(address) ──→ HttpServerListener.onAfterRestart(address)
        │
        └─ [Failure: IOException]
            ├─ notifyStartFailure(e) ──→ HttpServerListener.onStartFailure(e)
            ├─ Check shouldRetry(attempt, e)
            │   ├─ [false] → Throw RuntimeException
            │   └─ [true] → Sleep(backoffMs), attempt++, retry
            └─ [Retry succeeds] → notifyAfterRestart(address)

Later: HttpServerSupervisor.stop()
    ├─ server.stop()
    │   ├─ httpServer.stop(0) [graceful]
    │   ├─ httpServer = null
    │   └─ running = false
    └─ serverRunning = false
```

---

## Integration with Phase-6 Section 2 (Future)

The HttpServerListener interface will be implemented as an IPC notifier:

```java
public class HttpServerIPCNotifier implements HttpServerListener {
    private final IMessageChannel ipcChannel
    
    @Override
    public void onBeforeRestart() {
        ipcChannel.send("""
            {
                "type": "HTTP_SERVER_RESTARTING",
                "timestamp": "2026-01-09T..."
            }
        """)
    }
    
    @Override
    public void onAfterRestart(String newAddress) {
        ipcChannel.send("""
            {
                "type": "HTTP_SERVER_RESTARTED",
                "address": "127.0.0.1:54321",
                "timestamp": "2026-01-09T..."
            }
        """)
    }
    
    @Override
    public void onStartFailure(Throwable error) {
        ipcChannel.send("""
            {
                "type": "HTTP_SERVER_FAILED",
                "error": "IOException: ...",
                "timestamp": "2026-01-09T..."
            }
        """)
    }
}
```

---

## Error Handling Strategy

### Start Errors
- Invalid path → `IOException` thrown
- Bind failure (port in use) → `IOException` thrown
- Network error → `IOException` caught and retried

### Stop Errors
- Not running → `IllegalStateException`
- Already stopped → `IllegalStateException`

### Listener Errors
- Listener throws exception → Caught and logged, does not propagate
- All listeners invoked even if one fails

### File Serving Errors
- Invalid path (directory traversal) → 400 Bad Request
- File not found → Fallback to index.html
- Index not found → 404 Not Found
- Non-GET method → 405 Method Not Allowed
- Read failure → 500 Internal Server Error

---

## Key Design Decisions

### ✅ Ephemeral Port
- Port 0 tells OS to assign an available port
- Actual port queried after bind via `httpServer.getAddress().getPort()`
- Useful for testing and scenarios where port conflicts occur

### ✅ Localhost-Only Binding
- `InetSocketAddress("127.0.0.1", port)` ensures no external access
- Security without Phase-7 hardening

### ✅ Exponential Backoff
- Prevents thundering herd on repeated failures
- Simple formula: 100ms × 2^(n-1)
- Configurable via RetryPolicy interface

### ✅ Listener Pattern
- Passive observers (methods return void)
- No state mutations from listeners
- Snapshot taken before notification (safe concurrent modification)

### ✅ SPA Fallback
- Non-existent paths route to index.html
- Enables client-side routing
- Common pattern in modern web applications

### ✅ No External Dependencies
- Uses JDK `com.sun.net.httpserver.HttpServer`
- No Apache HttpComponents, Jetty, or Undertow
- Minimal, clean, focused

---


