# Phase-6 Section 1: VuePress HTTP Server Implementation

## Overview

This document describes the Phase-6 Section 1 implementation: the embedded HTTP server that serves prebuilt VuePress static documentation.

## Architecture Summary

```
┌─────────────────────────────────────────────────┐
│         Application Entry Point                  │
└────────────────────┬────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────┐
│  HttpServerSupervisor                            │
│  (Manages lifecycle, retries, notifications)     │
└────────────────┬────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────┐
│  VuePressHttpServer                              │
│  (Core HTTP server: start/stop, request routing) │
├─────────────────────────────────────────────────┤
│  StaticFileHandler                               │
│  (Serves files, SPA fallback to index.html)     │
└─────────────────────────────────────────────────┘
```

## Files Created

### Phase-6 Core Classes (Production Code)

1. **HttpServerConfig.java** (`com.ui.cef_control.http`)
   - Immutable configuration object
   - Properties: `staticFilesPath`, `port` (0 = ephemeral), `bindAddress`
   - Validation: Non-empty paths, valid ports, non-empty bind address

2. **HttpServerListener.java** (`com.ui.cef_control.http`)
   - Interface for lifecycle notifications
   - Methods:
     - `onBeforeRestart()` - Called before server restart
     - `onAfterRestart(String newAddress)` - Called after restart with new address
     - `onStartFailure(Throwable error)` - Called on startup failure

3. **VuePressHttpServer.java** (`com.ui.cef_control.http`)
   - Core HTTP server implementation
   - Key methods:
     - `start()` - Binds to 127.0.0.1 on configured port, registers handler
     - `stop()` - Graceful shutdown
     - `getActualAddress()` - Returns "127.0.0.1:PORT" (useful after ephemeral port)
     - `getActualPort()` - Returns bound port number
     - `isRunning()` - Current running state
     - `addListener()` / `removeListener()` - Lifecycle notification registration
   - Handler setup: StaticFileHandler mapped to "/" context
   - Uses: `com.sun.net.httpserver.HttpServer` (JDK built-in, no external dependency)

4. **StaticFileHandler.java** (`com.ui.cef_control.http`)
   - HTTP request handler for static file serving
   - Routing logic:
     - GET /path/to/file.ext → Serve if exists
     - GET /nonexistent/path → Fallback to index.html (SPA support)
     - Other methods (POST, PUT, DELETE) → 405 Method Not Allowed
     - Directory traversal attempts (../) → 400 Bad Request
   - Content-Type detection: Automatic based on file extension
   - No caching, compression, or security headers (Phase-7 concerns)

5. **HttpServerSupervisor.java** (`com.ui.cef_control.http`)
   - Supervisor for HTTP server lifecycle management
   - Responsibilities:
     - Start with automatic retry on failure (exponential backoff)
     - Stop server gracefully
     - Maintain running state
     - Notify listeners before and after restarts
     - Use RetryPolicy for retry logic
   - Key methods:
     - `start()` - Attempt startup with retries, notify listeners
     - `stop()` - Shutdown
     - `isServerRunning()` - Query state
     - `getServerAddress()` / `getServerPort()` - Get current binding
     - `addListener()` / `removeListener()` - Register for notifications

### Phase-6 Extended Classes

6. **RetryPolicy.java** (`com.ui.cef_control.supervisor`) - **EXTENDED**
   - Added: `getBackoffMs(int attempt)` default method
   - Exponential backoff: 100ms * 2^(attempt-1)
   - Example: attempt 1→100ms, attempt 2→200ms, attempt 3→400ms, etc.

### Test Classes

7. **VuePressHttpServerTest.java** (`com.ui.cef_control.http.test`)
   - Tests: start/stop, listener notifications, ephemeral port handling
   - Validates: Config validation, error handling, listener isolation

8. **HttpServerSupervisorTest.java** (`com.ui.cef_control.http.test`)
   - Tests: Supervisor start/stop, listener notifications, retry policy invocation
   - Validates: State management, error handling

9. **StaticFileHandlerTest.java** (`com.ui.cef_control.http.test`)
   - Tests: Static file serving, SPA fallback, method rejection, directory traversal prevention
   - Uses: Custom TestHttpExchange (no external mocking framework)

## Lifecycle Flow

### Server Startup
```
Application
    ↓
HttpServerSupervisor.start()
    ↓ (notifyBeforeRestart)
HttpServerListener.onBeforeRestart()
    ↓
VuePressHttpServer.start()
    ├─ Validate static files path exists
    ├─ Create HttpServer bound to 127.0.0.1:{port}
    ├─ Register StaticFileHandler on "/" context
    └─ Call server.start()
    ↓ (notifyAfterRestart with new address)
HttpServerListener.onAfterRestart("127.0.0.1:54321")
    ↓
Server now accepting requests

```

### Restart on Crash (Future Integration)
```
External detection (e.g., periodic health check)
    ↓
HttpServerSupervisor.start() [retry loop]
    ↓ (attempt 1 fails)
HttpServerListener.onStartFailure(IOException)
    ↓
Sleep(backoffMs from RetryPolicy)
    ↓ (attempt 2 succeeds)
notifyBeforeRestart()
    ↓
notifyAfterRestart("127.0.0.1:NEW_PORT")
    ↓
Server running on new port
```

### Graceful Shutdown
```
HttpServerSupervisor.stop()
    ↓
VuePressHttpServer.stop()
    ├─ Call httpServer.stop(0) [wait for in-flight requests]
    └─ Set running = false
    ↓
Server stops accepting new requests
Existing connections drain
```

## Design Constraints (Phase-6 Only)

### ✅ IMPLEMENTED
- Serve prebuilt VuePress static files
- Bind ONLY to 127.0.0.1 (localhost)
- Use ephemeral port (0 = OS-assigned)
- Restart server with listener notifications
- Java owns lifecycle (API is Java-controlled)
- Simple exponential backoff for retries

### ❌ DEFERRED TO PHASE-7
- HTTPS/TLS support
- Authentication/authorization
- Caching headers (Cache-Control, ETag, If-Modified-Since)
- Compression (gzip, brotli)
- Rate limiting
- Request logging/metrics
- Security headers (HSTS, CSP, X-Frame-Options, etc.)
- Service discovery / health checks
- Load balancing
- Session management

## Integration Points (Not Yet Implemented)

### Phase-6 Section 2 (Future)
- Implement IPC channel to send `onBeforeRestart()` and `onAfterRestart()` notifications to CEF
- Enable CEF to pause/resume operations during restart

### Phase-7 (Explicitly Deferred)
- Integrate with ProcessController for lifecycle coordination
- Add authentication layer
- Implement metrics/monitoring
- Add hardening (security headers, input validation, rate limiting)

## Testing Strategy

### Unit Tests
- **VuePressHttpServerTest**: Core server functionality, config validation
- **HttpServerSupervisorTest**: Retry logic, listener notifications, state transitions
- **StaticFileHandlerTest**: File routing, SPA fallback, method rejection, directory traversal prevention

### Integration Tests (Not Yet Implemented)
- End-to-end HTTP requests to running server
- Restart scenarios with listener integration
- Port reuse after crash scenarios

## Dependencies

- **JDK 11+**: Built-in `com.sun.net.httpserver.HttpServer`
- **Testing**: JUnit 4 (already in pom.xml)
- **No external HTTP frameworks**: Uses only JDK built-ins

## Configuration Example

```java
// Create config for ephemeral port on localhost
HttpServerConfig config = new HttpServerConfig(
    "/path/to/vuepress/build/.vuepress/dist", // Static files directory
    0,                                          // 0 = ephemeral (OS-assigned) port
    "127.0.0.1"                                // Bind to localhost only
);

// Create server
VuePressHttpServer server = new VuePressHttpServer(config);

// Create supervisor with retry policy
HttpServerSupervisor supervisor = new HttpServerSupervisor(
    server,
    RetryPolicy.maxRetries(3)  // Retry up to 3 times before giving up
);

// Add listener for restart notifications (will be replaced with IPC in Phase-6 Section 2)
supervisor.addListener(new HttpServerListener() {
    @Override
    public void onBeforeRestart() {
        System.out.println("Server restarting...");
    }
    
    @Override
    public void onAfterRestart(String newAddress) {
        System.out.println("Server now available at: " + newAddress);
    }
    
    @Override
    public void onStartFailure(Throwable error) {
        System.err.println("Failed to start server: " + error);
    }
});

// Start the server (with retries)
supervisor.start();

// Later: stop gracefully
supervisor.stop();
```

## File Structure

```
src/main/java/com/ui/cef_control/http/
├── HttpServerConfig.java
├── HttpServerListener.java
├── VuePressHttpServer.java
├── StaticFileHandler.java
└── HttpServerSupervisor.java

src/test/java/com/ui/cef_control/http/
├── VuePressHttpServerTest.java
├── HttpServerSupervisorTest.java
└── StaticFileHandlerTest.java

src/main/java/com/ui/cef_control/supervisor/
└── RetryPolicy.java (EXTENDED with getBackoffMs)
```

## Compliance with Phase-6 Constraints

✅ **Do not redesign existing abstractions**: No changes to ProcessController, UISupervisor, or IPC layer.

✅ **Do not introduce Phase-7 logic**: No HTTPS, auth, metrics, caching, compression, or hardening.

✅ **Do not move authority away from Java**: HttpServerListener is passive; Java controls lifecycle.

✅ **Do not add business logic in JS**: No JavaScript involved; purely backend HTTP serving.

✅ **Explicitly defer Phase-7**: All Phase-7 concerns documented above and deferred.

## Next Steps

1. **Phase-6 Section 2**: Implement IPC channel to notify CEF of restart events
2. **Phase-6 Section 3**: Integrate HTTP server start/stop with CEF process lifecycle
3. **Testing**: Run full test suite to validate functionality
4. **Phase-7**: Add HTTPS, auth, metrics, security hardening


