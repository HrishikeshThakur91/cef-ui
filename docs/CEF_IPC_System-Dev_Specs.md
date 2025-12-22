# CEF UI + IPC SYSTEM — FULL SYSTEM SPECIFICATION

    - Scope: End-to-end system
    - Covers: Phase 1 → Phase 7
    - Target OS: Windows
    - Languages: 
        - Core + UI Host: C++ (64-bit)
        - Backend App: Java (32-bit, existing)
        - UI Stack: CEF + VuePress
        - IPC: Secure WebSockets (WSS, localhost)
        - Methodology: Clean Architecture + TDD

# 1. SYSTEM GOAL

    - Build a secure, standalone CEF-based UI process that:
    - Renders VuePress-based documentation / UI
    - Communicates bidirectionally with an existing 32-bit Java application
    - Runs as a separate 64-bit process
    - Uses secure IPC (WSS over localhost)
    - Can be started, stopped, and restarted by the Java application
    - Works on locked-down enterprise machines

# 2. HIGH-LEVEL SYSTEM VIEW
┌──────────────────────────────────────────────┐
│           Java Application (32-bit)          │
│                                              │
│  - Hosts HTTPS/WSS server (Spring)           │
│  - Controls lifecycle of UI process          │
│  - Provides session token                    │
│  - Serves VuePress UI content                │
└───────────────▲──────────────────────────────┘
                │   WSS (localhost, TLS)
                │
┌───────────────┴──────────────────────────────┐
│        CEF UI Process (64-bit, C++)          │
│                                              │
│  - IPC Core (Phases 1–5)                     │
│  - CEF Browser integration                   │
│  - VuePress UI rendering                     │
│  - Multi-window support                      │
└──────────────────────────────────────────────┘

# 3. ARCHITECTURAL PRINCIPLES

## Core Principles

    - Process isolation: Java and UI are separate executables
    - Policy ≠ Transport: Retry & lifecycle logic is independent of IPC transport
    - Fail fast: Invalid config, protocol violations throw immediately
    - Secure by default: TLS everywhere, no bypass flags
    - Test-driven: All behavior enforced by tests

## Explicit Non-Goals
    - No embedded browser inside Java
    - No JNI / JCEF inside Java
    - No OpenSSL
    - No async threading models initially
    - No global singletons

# 4. PHASED SYSTEM ARCHITECTURE

## PHASE 1 — Configuration & Startup

### Components

    - AppConfig

### Responsibilities

    - Parse CLI args from Java launcher
    - Validate:
        - IPC port
        - Session token
        - HTTPS start URL
        - Window ID / instance ID
    - Reject unknown flags

### Output

    - Immutable runtime configuration object

## PHASE 2 — Process Lifecycle

### Components

    - ProcessController

### Responsibilities

    - Manage process states:

>        Stopped → Starting → Running → Stopping

    - Notify listeners of lifecycle events
    - Ensure deterministic shutdown

### Notes

    - Single-threaded
    - No OS process management here (handled by Java)

## PHASE 3 — IPC Protocol & Contracts

### Message Model

    - JSON messages
    - Explicit type field
    - Strict parsing (fail on malformed messages)

### Handshake

    - UI sends HELLO
    - Java validates session token
    - Token comparison:
        - Leading/trailing whitespace trimmed (incoming only)
        - Token treated as opaque value

### IMessageChannel

> Send(string)
> Receive() -> string
> IsConnected() -> bool
> Close()
> 
> 
> No explicit Connect() — channel is assumed usable or failing.

## PHASE 4 — IPC Transport & Retry Policy

### WebSocketTransport

    - Binds WebSockets to IMessageChannel
    - Requests TLS context from provider
    - Manages connected state
    - No retries, no timers

### WssConnectionManager

    - Owns retry & backoff logic
    - Retry strategy:
        - 1s → 2s → 4s → 8s (cap)
        - Max 5 retries
    - Uses:
>        
>       Send("")  // triggers underlying connect or failure
> 

    - Emits connection lifecycle events

## PHASE 5 — Real WSS Implementation

### Components

    - BeastWebSocketConnection

### Technology

    - Boost.Asio
    - Boost.Beast
    - Windows TLS (SChannel)

### Responsibilities

    - Establish WSS connection
    - Validate server certificate via OS store
    - Send / receive text frames
    - Convert transport errors → IpcProtocolException
    
### Constraints
    
    - Synchronous only
    - No retries
    - No threading
    - One connection per instance

## PHASE 6 — CEF INTEGRATION

### Components

    - CefApp
    - CefBrowserProcessHandler
    - CefClient
    - CefLifeSpanHandler
    - CefDisplayHandler

### Responsibilities

#### Process Initialization

    - Initialize CEF in UI process only
    - Use off-the-shelf CEF binaries (64-bit)
    - Disable unnecessary features:
        - GPU (optional, based on stability)
        - Extensions
        - Remote debugging (unless explicitly enabled)

#### Browser Creation

    - One browser per window
    - Each browser bound to:
        - Unique window ID
        - IPC session token
    - URL Loading
    - Load VuePress UI from:  " https://localhost:<port>/<path> "
    - Must use HTTPS (enterprise machines block HTTP)

## PHASE 7 — UI ↔ APPLICATION FUNCTIONALITY

### Java → UI (Control)

    - Start UI process
    - Restart UI process
    - Close UI window
    - Notify UI of context changes

### UI → Java (Requests)

    - Navigation requests
    - Page / section resolution
    - “Not found” fallback requests
    - Error reporting

### Example Flow

    - Java launches UI with config
    - UI connects via WSS
    - Handshake completes
    - UI loads VuePress page
    - User action triggers IPC message
    - Java responds with navigation directive
    - UI updates browser state

----------------------------------- Phase 2 --------------------------------

# 5. MULTI-WINDOW SUPPORT

## Design

    - One UI process may host multiple CEF windows
    - Each window:
        - Has its own browser instance
        - Shares IPC channel (initially)
    - Window ID passed at startup

## Future Extension

    - One IPC channel per window (optional)
    - Window-specific routing

# 6. SECURITY MODEL (END-TO-END)

## Transport

    - WSS only
    - OS certificate trust
    - No insecure flags
    - No certificate pinning initially

## Authentication

    - Session token (Java-generated)
    - Validated during handshake
    - Not reused across sessions

## Environment

    - Localhost only
    - Firewall-friendly
    - Enterprise proxy-safe

# 7. ERROR HANDLING STRATEGY

 ___________________________________________________
|      Error        |          Types                |
|-------------------|-------------------------------|
|   Layer	        |   Error                       |
|   Config          |	InvalidConfigException      |
|   IPC / Protocol  |	IpcProtocolException        |
|   Transport       |	Wrapped as IPC exception    |
|   CEF             |	Logged + graceful shutdown  |
 ---------------------------------------------------

## Strategy

    - Fail fast
    - Retry only at policy layer
    - Never silently ignore failures

# 8. TEST STRATEGY (FULL SYSTEM)

## Unit Tests

    - Config
    - Lifecycle
    - Protocol parsing
    - Retry logic
    - Transport abstraction

## Integration Tests

    - Real WSS loopback server
    - BeastWebSocketConnection

## Manual / System Tests

    - Java launches UI
    - Cert installation verification
    - UI restart scenarios
    - Network failure recovery

# 9. INSTALLER & DEPLOYMENT

## Packaging

    - Separate installers:
        - Java app
        - CEF UI app
    - UI installer includes:
        - CEF binaries
        - UI executable
        - Dependencies

## Certificates

    - Java HTTPS cert installed in:
        - Windows Trusted Root (admin)
    - UI relies on OS trust store

# 10. OPERATIONAL CONSIDERATIONS

    - Locked-down enterprise machines
    - No user-level cert bypass
    - No dynamic port scanning
    - Hybrid port strategy (fixed → dynamic fallback)

# 11. FUTURE EXTENSIONS (OUT OF SCOPE)

    - Async IPC
    - Message batching
    - Binary protocol
    - Plugin system
    - Headless UI mode

# 12. IMPLEMENTATION CHECKPOINTS

 _______________________________
|   Phase       |	Status      |
|---------------|---------------|
|   Phase 1–4   |	Complete    |
|   Phase 5     |	Started     |
|   Phase 6     |	Planned     |
|   Phase 7     |	Planned     |
 -------------------------------

 # 13. GLOSSARY
    - CEF: Chromium Embedded Framework
    - WSS: WebSocket Secure
    - IPC: Inter-Process Communication
    - TLS: Transport Layer Security
    - JSON: JavaScript Object Notation
    - UI: User Interface
    - CLI: Command-Line Interface
    - OS: Operating System