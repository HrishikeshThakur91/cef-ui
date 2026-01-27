# Phase-6 vs Phase-7: Explicit Deferral Map

## Overview

This document explicitly marks every feature, method, and design decision as either **Phase-6** (implemented) or **Phase-7** (deferred).

---

## HTTP Server Features

### Phase-6 ✅ IMPLEMENTED

| Feature | Location | Status | Notes |
|---------|----------|--------|-------|
| Serve static files | StaticFileHandler.handle() | ✅ | Reads from configured directory |
| SPA routing | StaticFileHandler.handle() | ✅ | Unknown paths → index.html |
| Localhost-only binding | VuePressHttpServer.start() | ✅ | 127.0.0.1 hardcoded |
| Ephemeral port support | HttpServerConfig | ✅ | Port 0 = OS-assigned |
| GET request handling | StaticFileHandler.handle() | ✅ | GET only, others → 405 |
| Directory traversal prevention | StaticFileHandler.handle() | ✅ | Rejects paths with ".." |
| Server lifecycle | VuePressHttpServer.start/stop | ✅ | Start validates path, stop graceful |
| Listener notifications | HttpServerSupervisor | ✅ | onBeforeRestart, onAfterRestart, onStartFailure |
| Retry with backoff | HttpServerSupervisor.start() | ✅ | Exponential: 100ms × 2^(n-1) |
| State management | VuePressHttpServer.running | ✅ | isRunning(), actual port/address tracking |
| Content-Type detection | StaticFileHandler.guessContentType() | ✅ | Extension-based + Files.probeContentType() |
| Graceful shutdown | VuePressHttpServer.stop() | ✅ | httpServer.stop(0) with indefinite wait |

### Phase-7 ❌ DEFERRED

| Feature | Why Deferred | Phase-7 Approach |
|---------|--------------|------------------|
| HTTPS/TLS | Security concern | Add HttpsServer, keystore management, cert handling |
| Authentication | Security concern | Add Authorization header parsing, JWT validation, session mgmt |
| Caching headers | Performance concern | Add Cache-Control, ETag, If-Modified-Since, Last-Modified |
| Compression | Performance concern | Add gzip/brotli with Content-Encoding negotiation |
| Rate limiting | Security concern | Add request counting, token bucket, 429 responses |
| Request logging | Operational concern | Add structured logging, metrics, tracing |
| Security headers | Security concern | Add HSTS, CSP, X-Frame-Options, X-Content-Type-Options |
| Health checks | Operational concern | Add /health endpoint, liveness/readiness probes |
| Service discovery | Operational concern | Add registration with service mesh, consul, etc. |
| Load balancing | Operational concern | Multiple instances, round-robin, failover |
| Session management | Application concern | Add session cookies, token storage, user tracking |
| Input validation | Security concern | Add request size limits, content-type enforcement, path canonicalization |
| Error pages | UX concern | Add custom 404/500 HTML pages, error details in response |
| CORS headers | Security concern | Add Access-Control-* headers, preflight handling |
| IPv6 support | Compatibility concern | Support :: and [::1] binding |
| HTTP/2 support | Protocol concern | Upgrade to Http2Server or add h2 support |
| WebSocket support | Protocol concern | Add WebSocket handler, upgrade negotiation |
| Range requests | Performance concern | Add Content-Range, 206 Partial Content |
| Conditional requests | Performance concern | Add 304 Not Modified, conditional headers |
| Custom MIME types | Configuration concern | Add .mimeTypes config file, dynamic loading |

---

## Class-by-Class Phase Annotation

### HttpServerConfig

```
CLASS: HttpServerConfig
PHASE: 6 ✅

METHODS:
  constructor(staticFilesPath, port, bindAddress)     PHASE-6 ✅
  getStaticFilesPath()                                 PHASE-6 ✅
  getPort()                                            PHASE-6 ✅
  getBindAddress()                                     PHASE-6 ✅

DEFERRED TO PHASE-7:
  - SSL/TLS keystore configuration
  - Default MIME types configuration
  - Connection timeout settings
  - Max request size limits
  - Compression settings
```

### HttpServerListener

```
CLASS: HttpServerListener (Interface)
PHASE: 6 ✅

METHODS:
  onBeforeRestart()                                    PHASE-6 ✅
  onAfterRestart(String newAddress)                   PHASE-6 ✅
  onStartFailure(Throwable error)                     PHASE-6 ✅

DEFERRED TO PHASE-7:
  - onHealthCheckFailed(Throwable error)
  - onMetricsUpdated(Metrics m)
  - onSecurityEvent(String event)
  - onCompressionStatusChanged(boolean enabled)
```

### VuePressHttpServer

```
CLASS: VuePressHttpServer
PHASE: 6 ✅

METHODS:
  constructor(HttpServerConfig)                       PHASE-6 ✅
  start() throws IOException                          PHASE-6 ✅
    - Validates path existence                        PHASE-6 ✅
    - Binds to 127.0.0.1:port                         PHASE-6 ✅
    - Registers StaticFileHandler                     PHASE-6 ✅
    - Calls httpServer.start()                        PHASE-6 ✅
  stop()                                              PHASE-6 ✅
    - Calls httpServer.stop(0) graceful               PHASE-6 ✅
  getActualAddress() → String                         PHASE-6 ✅
  getActualPort() → int                               PHASE-6 ✅
  isRunning() → boolean                               PHASE-6 ✅
  addListener(HttpServerListener)                     PHASE-6 ✅
  removeListener(HttpServerListener)                  PHASE-6 ✅

DEFERRED TO PHASE-7:
  - getMetrics() → ServerMetrics
  - setMaxConnections(int)
  - setRequestTimeout(Duration)
  - getConnectionCount() → int
  - enableSSL(SSLContext)
  - enableCompression(CompressionType)
  - setErrorHandler(ErrorHandler)
  - getSecurityStatus() → SecurityStatus
  - healthCheck() → HealthStatus
  - enableLogging(Logger)
```

### StaticFileHandler

```
CLASS: StaticFileHandler
PHASE: 6 ✅

METHODS:
  constructor(Path staticRoot)                        PHASE-6 ✅
  handle(HttpExchange) throws IOException             PHASE-6 ✅
    - Checks GET method                               PHASE-6 ✅
    - Validates path (no ..)                          PHASE-6 ✅
    - Serves requested file or index.html             PHASE-6 ✅
    - Returns 405 for non-GET                         PHASE-6 ✅
    - Returns 400 for path traversal                  PHASE-6 ✅
    - Returns 404 for missing file                    PHASE-6 ✅
  serveFile(HttpExchange, Path)                       PHASE-6 ✅
    - Reads file content                              PHASE-6 ✅
    - Sets Content-Type header                        PHASE-6 ✅
    - Sends 200 with content                          PHASE-6 ✅
  guessContentType(Path) → String                     PHASE-6 ✅
    - Extension-based detection                       PHASE-6 ✅
    - Files.probeContentType() fallback               PHASE-6 ✅

DEFERRED TO PHASE-7:
  - handle(HttpExchange, RequestContext) with auth
  - validateRequest(HttpExchange, SecurityContext)
  - applySecurityHeaders(HttpExchange)
  - handleRangeRequest(HttpExchange, Path)
  - handleConditionalRequest(HttpExchange, Path)
  - compressResponse(HttpExchange, byte[])
  - cacheFile(Path) → CachedFile
  - logRequest(HttpExchange)
  - validateContentLength(HttpExchange)
  - handleCORS(HttpExchange)
  - getSecurityContext() → SecurityContext
  - setCustomErrorPage(int code, Path errorPage)
```

### HttpServerSupervisor

```
CLASS: HttpServerSupervisor
PHASE: 6 ✅

METHODS:
  constructor(VuePressHttpServer, RetryPolicy)       PHASE-6 ✅
  start()                                             PHASE-6 ✅
    - Calls notifyBeforeRestart()                     PHASE-6 ✅
    - Calls server.start()                            PHASE-6 ✅
    - Retries with retryPolicy                        PHASE-6 ✅
    - Exponential backoff via getBackoffMs()          PHASE-6 ✅
    - Calls notifyAfterRestart() on success           PHASE-6 ✅
    - Calls notifyStartFailure() on each failure      PHASE-6 ✅
  stop()                                              PHASE-6 ✅
  isServerRunning() → boolean                         PHASE-6 ✅
  getServerAddress() → String                         PHASE-6 ✅
  getServerPort() → int                               PHASE-6 ✅
  addListener(HttpServerListener)                     PHASE-6 ✅
  removeListener(HttpServerListener)                  PHASE-6 ✅

DEFERRED TO PHASE-7:
  - start(Duration timeout)
  - startWithHealthCheck()
  - orchestrateGracefulShutdown(Duration timeout)
  - getSupervisionMetrics() → SupervisorMetrics
  - enableMonitoring(Monitor m)
  - setRetryPolicyDynamic(RetryPolicy, Condition)
  - pauseAutoRestart()
  - resumeAutoRestart()
  - getRetryHistory() → List<RetryAttempt>
  - registerMetricsCollector(MetricsCollector)
```

### RetryPolicy (Extended)

```
INTERFACE: RetryPolicy
PHASE: Extended for Phase-6 ✅

EXISTING METHODS:
  shouldRetry(int attempt, Throwable failure) → boolean  [PHASE-5]

NEW PHASE-6 METHODS:
  getBackoffMs(int attempt) → long                    PHASE-6 ✅
    - Default: 100ms × 2^(attempt-1)
    - Override for custom backoff

DEFERRED TO PHASE-7:
  - getBackoffMs(int attempt, Throwable failure)
  - getMaxRetries() → int
  - isRetryableError(Throwable) → boolean
  - getJitterMs(int attempt) → long
  - shouldResetCounter() → boolean
  - getCircuitBreakerThreshold() → int
```

---

## Listener Implementation Pattern

### Phase-6 Simple Listener (Current)

```java
// Phase-6: Simple console logging
server.addListener(new HttpServerListener() {
    @Override
    public void onBeforeRestart() {
        System.out.println("Server restarting...");  // PHASE-6
    }
    
    @Override
    public void onAfterRestart(String newAddress) {
        System.out.println("Now at: " + newAddress);  // PHASE-6
    }
    
    @Override
    public void onStartFailure(Throwable error) {
        System.err.println("Failed: " + error);  // PHASE-6
    }
});
```

### Phase-6 Section 2: IPC Notifier (Next)

```java
// Phase-6 Section 2: IPC-based notifications
class HttpServerIPCNotifier implements HttpServerListener {
    private IMessageChannel ipcChannel;
    
    @Override
    public void onBeforeRestart() {
        ipcChannel.send("""
            {"type": "HTTP_RESTART", "status": "starting"}
        """);  // PHASE-6
    }
    
    @Override
    public void onAfterRestart(String newAddress) {
        ipcChannel.send("""
            {"type": "HTTP_RESTART", "status": "done", "url": "...:..."}
        """);  // PHASE-6
    }
    
    @Override
    public void onStartFailure(Throwable error) {
        ipcChannel.send("""
            {"type": "HTTP_RESTART", "status": "failed", "error": "..."}
        """);  // PHASE-6
    }
}
```

### Phase-7: Full-Featured Listener (Deferred)

```java
// Phase-7: Metrics, auth, security
class HttpServerFullFeaturedListener implements HttpServerListener {
    private IMessageChannel ipcChannel;
    private MetricsCollector metrics;
    private SecurityAudit audit;
    
    @Override
    public void onBeforeRestart() {
        // PHASE-7: Check auth before notifying
        if (!securityContext.hasPermission("server.restart")) {
            audit.log("UNAUTHORIZED_RESTART_ATTEMPT");
            throw new SecurityException();
        }
        
        // PHASE-7: Record metrics
        metrics.increment("server.restarts");
        metrics.gauge("restart.timestamp", System.currentTimeMillis());
        
        // PHASE-7: Send encrypted IPC with signature
        ipcChannel.send(encryptAndSign({"type": "HTTP_RESTART", ...}));
    }
    
    // ... similar for onAfterRestart, onStartFailure
}
```

---

## Request Handling Flow: Phase-6 vs Phase-7

### Phase-6 Request Flow (Current)

```
HttpExchange received
    ↓
StaticFileHandler.handle()
    ├─ [Check GET method] → PHASE-6 ✅
    ├─ [Extract path] → PHASE-6 ✅
    ├─ [Check directory traversal] → PHASE-6 ✅
    ├─ [Resolve file in static root] → PHASE-6 ✅
    ├─ [Serve file or index.html] → PHASE-6 ✅
    └─ [Set Content-Type, send 200] → PHASE-6 ✅
```

### Phase-7 Request Flow (Deferred)

```
HttpExchange received
    ↓
[PHASE-7] SecurityFilter.authenticate()
    ├─ Check Authorization header
    ├─ Validate JWT/session token
    └─ Establish SecurityContext
    ↓
[PHASE-7] SecurityFilter.authorize()
    ├─ Check user permissions
    └─ Enforce rate limiting
    ↓
[PHASE-7] LoggingFilter.logRequest()
    └─ Log HTTP method, path, headers
    ↓
StaticFileHandler.handle()  [PHASE-6 core logic]
    ├─ [Check GET method]
    ├─ [Extract and validate path]
    ├─ [Serve file]
    └─ [Set response headers]
    ↓
[PHASE-7] CachingFilter.applyHeaders()
    ├─ Add Cache-Control
    ├─ Add ETag
    └─ Add Last-Modified
    ↓
[PHASE-7] CompressionFilter.compress()
    ├─ Detect Accept-Encoding
    └─ Compress with gzip/brotli
    ↓
[PHASE-7] SecurityFilter.applyHeaders()
    ├─ Add HSTS
    ├─ Add CSP
    ├─ Add X-Frame-Options
    └─ Add X-Content-Type-Options
    ↓
[PHASE-7] LoggingFilter.logResponse()
    ├─ Log status code, size, duration
    └─ Record metrics
    ↓
Send response to client
```

---

## Testing: Phase-6 vs Phase-7

### Phase-6 Tests ✅ IMPLEMENTED

| Test | Class | Method | Phase |
|------|-------|--------|-------|
| Start server | VuePressHttpServerTest | testServerStartsSuccessfully | 6 ✅ |
| Double-start prevention | VuePressHttpServerTest | testServerCannotStartTwice | 6 ✅ |
| Stop server | VuePressHttpServerTest | testServerStopsSuccessfully | 6 ✅ |
| Double-stop prevention | VuePressHttpServerTest | testServerCannotStopIfNotRunning | 6 ✅ |
| Listener notification | VuePressHttpServerTest | testListenerNotifiedOfStart | 6 ✅ |
| Failure notification | VuePressHttpServerTest | testListenerNotifiedOfFailure | 6 ✅ |
| Ephemeral port | VuePressHttpServerTest | testActualPortAfterEphemeralStart | 6 ✅ |
| Address format | VuePressHttpServerTest | testActualAddressFormat | 6 ✅ |
| Listener removal | VuePressHttpServerTest | testRemoveListenerPreventsNotification | 6 ✅ |
| Supervisor start | HttpServerSupervisorTest | testSupervisorStartsServer | 6 ✅ |
| Supervisor stop | HttpServerSupervisorTest | testSupervisorStopsServer | 6 ✅ |
| Before-restart notify | HttpServerSupervisorTest | testSupervisorNotifiesBeforeRestart | 6 ✅ |
| Retry policy | HttpServerSupervisorTest | testRetryPolicyIsInvokedOnStartFailure | 6 ✅ |
| File serving | StaticFileHandlerTest | testServeIndexHtml | 6 ✅ |
| SPA fallback | StaticFileHandlerTest | testFallbackToIndexHtmlForNonExistentPath | 6 ✅ |
| Directory traversal prevention | StaticFileHandlerTest | testRejectDirectoryTraversal | 6 ✅ |
| Method rejection | StaticFileHandlerTest | testRejectPostRequest | 6 ✅ |
| Directory traversal via POST | StaticFileHandlerTest | testRejectPutRequest | 6 ✅ |
| Directory traversal via DELETE | StaticFileHandlerTest | testRejectDeleteRequest | 6 ✅ |

### Phase-7 Tests ❌ DEFERRED

- Authentication tests (JWT validation, session handling)
- Authorization tests (permission enforcement)
- Caching tests (ETag, If-Modified-Since, Cache-Control)
- Compression tests (gzip, brotli, content negotiation)
- Rate limiting tests (429 responses, token bucket)
- Security header tests (HSTS, CSP, X-Frame-Options)
- Logging/metrics tests (structured logs, metrics collection)
- HTTPS tests (certificate validation, TLS handshake)
- Health check tests (liveness, readiness)
- Load balancing tests (multiple instances, failover)
- IPv6 tests (binding to ::1, [::])
- HTTP/2 tests (multiplexing, server push)
- WebSocket tests (upgrade, framing)
- Range request tests (206 Partial Content)
- CORS tests (preflight, headers)
- Custom error page tests

---

## Configuration Examples: Phase-6 vs Phase-7

### Phase-6 Configuration

```java
// PHASE-6: Minimal, functional
HttpServerConfig config = new HttpServerConfig(
    "/var/www/vuepress/dist",
    0,              // ephemeral port
    "127.0.0.1"     // localhost only
);

VuePressHttpServer server = new VuePressHttpServer(config);
HttpServerSupervisor supervisor = new HttpServerSupervisor(
    server,
    RetryPolicy.maxRetries(3)  // 3 attempts
);

supervisor.addListener(new SimpleListener());  // logs only
supervisor.start();
```

### Phase-7 Configuration (Future)

```java
// PHASE-7: Feature-rich, secure, observable
HttpServerConfig config = new HttpServerConfig(
    "/var/www/vuepress/dist",
    0,
    "127.0.0.1"
);

// PHASE-7: Add SSL context
SSLContext sslContext = loadKeystore("/etc/ssl/keystore.jks");

// PHASE-7: Add compression settings
CompressionConfig compressionConfig = new CompressionConfig()
    .enableGzip()
    .enableBrotli()
    .setMinSize(1024);

// PHASE-7: Add cache settings
CacheConfig cacheConfig = new CacheConfig()
    .setMaxAge(Duration.ofDays(1))
    .setComputeETag(true)
    .setComputeLastModified(true);

// PHASE-7: Add security settings
SecurityConfig securityConfig = new SecurityConfig()
    .enableHSTS(Duration.ofDays(365))
    .setCSP("default-src 'self'")
    .setXFrameOptions("DENY")
    .requireAuthentication();

// PHASE-7: Add rate limiting
RateLimitConfig rateLimitConfig = new RateLimitConfig()
    .setRequestsPerSecond(1000)
    .setBurstSize(100);

// PHASE-7: Create advanced server
VuePressHttpServer server = new AdvancedVuePressHttpServer(config)
    .withSSLContext(sslContext)
    .withCompressionConfig(compressionConfig)
    .withCacheConfig(cacheConfig)
    .withSecurityConfig(securityConfig)
    .withRateLimitConfig(rateLimitConfig);

HttpServerSupervisor supervisor = new HttpServerSupervisor(
    server,
    RetryPolicy.exponentialBackoff(maxAttempts: 5, maxBackoff: Duration.ofMinutes(1))
);

// PHASE-7: Add multiple listeners
supervisor.addListener(new IPCNotifier());         // Send IPC
supervisor.addListener(new MetricsCollector());     // Collect metrics
supervisor.addListener(new SecurityAuditor());      // Log security events
supervisor.addListener(new HealthChecker());        // Monitor health
supervisor.addListener(new ErrorReporter());        // Report errors

supervisor.start();
```

---

## Summary Table

| Concern | Phase-6 | Phase-7 |
|---------|---------|---------|
| **Serving** | Static files only | + dynamic content (deferred) |
| **Security** | Directory traversal check only | + TLS, auth, rate limiting, headers |
| **Performance** | No optimizations | + Caching, compression, connection pooling |
| **Observability** | Listener notifications only | + Metrics, logging, distributed tracing |
| **Reliability** | Retry with backoff | + Health checks, circuit breaker, failover |
| **Protocol** | HTTP/1.1 only | + HTTP/2, WebSocket (deferred) |
| **Configuration** | Hardcoded localhost | + Dynamic, environment-based |
| **Testing** | Functional tests | + Performance, security, load tests |

---


