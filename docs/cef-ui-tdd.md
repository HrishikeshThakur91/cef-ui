# Global Rules for CEF UI Process TDD Implementation -

	- Do NOT auto-generate code for future phases
	- Do NOT scaffold multiple phases at once
	- Do NOT introduce placeholders for later phases
	- CEF code must live ONLY under src/ui and must not be referenced by core or ipc layers directly.

All code must be:

	- RAII-compliant
	- Exception-safe
	- Deterministic
	- Testable without UI
	- Free of undefined behavior

# CEF UI PROCESS (C++) — TDD IMPLEMENTATION SPEC

	- Platform: Windows x64
	- UI Engine: CEF (latest stable)
	- IPC: WSS (Java Spring backend)
	- Security: TLS + Session Handshake
	- Test Framework: GoogleTest
	- Approach: Phased TDD (NO SKIPS)

# GLOBAL RULES (MANDATORY)

Copilot MUST obey the following:

	- Tests first — always
	- One phase at a time
	- Do not import CEF before Phase 4
	- Do not open sockets before Phase 3
	- No certificate bypass flags
	- All tests must pass before next phase
	- No feature additions unless specified
	- Use dependency inversion for IPC & CEF

# PROJECT STRUCTURE ( Updated )

cef-ui/
├── cef-parallel/ <!--cef-ui Project-->
│   ├───inc
│   │   ├───core
│   │   │       AppConfig.h
│   │   │       ILifecycleListener.h
│   │   │       InvalidConfigException.h
│   │   │       Logger.h
│   │   │       ProcessController.h
│   │   ├───ipc
│   │   │       BeastWebSocketConnection.h
│   │   │       Handshake.h
│   │   │       IMessageChannel.h
│   │   │       IpcProtocolException.h
│   │   │       ITlsContextProvider.h
│   │   │       IWebSocketConnection.h
│   │   │       JsonParser.h
│   │   │       MessageTypes.h
│   │   │       WebSocketTransport.h
│   │   │       WssConnectionManager.h
│   │   └───ui
│   │           CefHandler.h
│   │           CefNativeAppl.h
│   │           CefUiBridge.h
│   │           CefUiBridgeImpl.h
│   │           resource.h
│   └───src
│      │   cef-ui.cpp
│      ├───core
│      │       AppConfig.cpp
│      │       ProcessController.cpp
│      ├───ipc
│      │       BeastWebSocketConnection.cpp
│      │       Handshake.cpp
│      │       JsonParser.cpp
│      │       WebSocketTransport.cpp
│      │       WssConnectionManager.cpp
│      └───ui
│            CefHandler.cpp
│            CefHandler_win.cpp
│            CefNativeAppl.cpp
│            CefUiBridgeImpl.cpp
├── cef-java-32
│   └───src
│      ├───main
│      │   └───java
│      │       └───com
│      │           └───ui
│      │               └───cef_control
│      │                   ├───config
│      │                   │       AppConfig.java
│      │                   │       InvalidConfigException.java
│      │                   ├───grpc
│      │                   │       CefControlServiceImpl.java
│      │                   │       CefServiceBootstrap.java
│      │                   │       cef_service.proto
│      │                   │       GrpcIpcServer.java
│      │                   │       GrpcMessageChannel.java
│      │                   │       HandshakeValidator.java
│      │                   ├───http
│      │                   │       DocsHttpServer.java
│      │                   │       DocsServerManager.java
│      │                   │       DocsState.java
│      │                   │       HttpServerConfig.java
│      │                   │       HttpServerListener.java
│      │                   │       HttpServerSupervisor.java
│      │                   │       StaticFileHandler.java
│      │                   │       VuePressHttpServer.java
│      │                   ├───ipc
│      │                   │       ConnectionListener.java
│      │                   │       Handshake.java
│      │                   │       IMessageChannel.java
│      │                   │       IpcConnection.java
│      │                   │       IpcMessage.java
│      │                   │       IpcServer.java
│      │                   │       MessageTypes.java
│      │                   │       WssChannel.java
│      │                   ├───lifecycle
│      │                   │       LifecycleListener.java
│      │                   │       ProcessController.java
│      │                   └───supervisor
│      │                           RetryPolicy.java
│      │                           SupervisorListener.java
│      │                           UIProcess.java
│      │                           UISupervisor.java
│      └───test
│         └───java
│            └───com
│               └───ui
│                  └───cef_control
│                     ├───config
│                     │       AppConfigTest.java
│                     ├───http
│                     │       HttpServerSupervisorTest.java
│                     │       StaticFileHandlerTest.java
│                     │       VuePressHttpServerTest.java
│                     ├───ipc
│                     │       HandshakeTest.java
│                     │       WssChannelTest.java
│                     ├───lifecycle
│                     │       ProcessControllerTest.java
│                     └───supervisor
│                           UISupervisorTest.java
│
│── cef-ui-tests/ <!--cef ui unit test Project-->
│   ├───inc
│   ├───src
│   │   │   test_main.cpp
│   │   ├───core
│   │   │       test_app_config.cpp
│   │   │       test_process_controller.cpp
│   │   ├───ipc
│   │   │       test_handshake.cpp
│   │   │       test_message_types.cpp
│   │   │       test_websocket_transport.cpp
│   │   │       test_wss_connection_manager.cpp
│   │   └───support
│   │           app_config_impl.cpp
│   │           ipc_impl.cpp
│   │           mock_connection_listener.h
│   │           mock_tls_context_provider.h
│   │           mock_websocket_connection.h
│   │           process_controller_impl.cpp
├── docs/
│   └──	cef-ui-tdd.md
└── README.md


# PHASE 1 — Configuration & Contracts (NO IPC, NO CEF)

## Goal

Establish immutable contracts and runtime configuration parsing.

### Write Tests FIRST

Create tests for:

	- AppConfig
		- Parses:
			-> --ipcPort
			-> --sessionToken
			-> --startUrl
			-> --windowId
		- Fails fast on missing values

Expected behavior :
> EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);

InvalidConfigException:

	- Must derive from std::runtime_error
	- Must include a clear error message

### Implement Minimal Code

Only after tests fail:

	- Implement AppConfig
	- No globals
	- No singletons

### Exit Criteria

	- All config tests pass
	- No IPC code
	- No CEF includes

# PHASE 2 — Process Lifecycle Controller

## Goal

Control start / shutdown / crash-safe exit without UI.

### Tests to Write

ProcessController
	
	- Can start	
	- Can shutdown gracefully	
	- Rejects double-start	
	- Emits lifecycle events

### Implementation Rules

	- No OS calls yet
	- Use mockable interfaces
	- No threads

### Exit Criteria

	- Deterministic lifecycle
	- No IPC
	- No UI

# PHASE 3 — ~~IPC CONTRACT (NO NETWORK YET)~~ <-> shift to Control Plane Contract (gRPC, NO RUNTIME)

## Goal

~~Define message schema & handshake logic without sockets.~~
Define gRPC service contract and semantic validation without starting gRPC runtime or threads.

### Message Schema (LOCKED)

~~HELLO~~
~~{~~
~~  "type": "HELLO",~~
~~  "sessionToken": "abc"~~
~~}~~

~~NAVIGATE~~
~~{~~
~~  "type": "NAVIGATE",~~
~~  "url": "/docs/page"~~
~~}~~
- .proto contract definition
- gRPC service + RPC definitions
- Semantic validation rules (not parsing rules)

### Tests to Write

~~- Handshake accepts valid token~~
~~- Rejects invalid token~~
~~- Unknown message rejected~~
~~- JSON parsing errors handled~~

- Handshake RPC accepts valid session token
- Handshake rejects invalid token
- Unknown RPC rejected by service definition
- Invalid request payload rejected

### Implementation Rules

~~- No sockets~~
~~- No threads~~
~~- Use interfaces only (IMessageChannel)~~

- No sockets
- No gRPC server/client startup
- .proto only
- No CEF includes

### Exit Criteria

~~- IPC logic testable~~
~~- No WebSocket~~
~~- No CEF~~

- .proto finalized
- Generated code compiles
- No runtime behavior

### Message parsing rules:

	- Unknown fields must be ignored
	- Missing required fields must reject the message
	- No dynamic casting

# PHASE 4 — ~~IPC TRANSPORT (WSS)~~ gRPC Transport (NO CEF)

## Goal

~~Connect to Java WSS securely.~~ 

- gRPC server/client wiring
- Channel lifecycle abstraction
- Retry / reconnect semantics

### Tests to Write

~~- Can connect to WSS endpoint~~
~~- TLS handshake succeeds~~
~~- HELLO sent on connect~~
~~- Reconnect on failure~~

- gRPC server starts
- gRPC client connects
- Handshake RPC invoked on connect
- Retry policy applied on failure

### Implementation Rules

~~- Use secure WebSocket library~~
~~- Certificate trust via OS store~~
~~- No insecure flags~~

- No CEF includes
- No UI logic
- gRPC threads isolated
- RetryPolicy used (already aligned with your recent change)

### Exit Criteria

~~- IPC works against mock server~~
~~- No UI yet~~

- Java ↔ CEF gRPC connection possible
- No UI interaction

# PHASE 5 — CEF BOOTSTRAP (UI MINIMAL)

## Goal

Start CEF, open window, load HTTPS URL.

### Tests to Write (Integration)

	- CEF initializes once
	- Window opens
	- URL loads
	- Clean shutdown

### Implementation Rules

	- Single window only
	- No JS binding yet
	- One UI thread

### Exit Criteria

	- CEF displays VuePress page
	- No IPC-UI interaction

# PHASE 6.1 - Java Control Plane Runtime (DONE, UNTESTED)

## Goal

Provide a runtime Java control plane capable of:
- Hosting or connecting to a gRPC endpoint
- Issuing control commands to the CEF process
- Supervising availability, retries, and restarts

This phase does NOT guarantee UI behavior, only control-plane reachability.

## Responsibilities

- Own gRPC channel lifecycle
- Own retry / backoff policy
- Own command issuance semantics
- Detect CEF availability / unavailability
- Cache last-intent commands for replay

## Required Runtime Capabilities

- Fire-and-forget unary RPC calls
- Retry with backoff using RetryPolicy
- Idempotent command re-issuance
- Graceful handling of:
	- connection loss
	- server restart
	- delayed availability

## Tests That SHOULD Exist (but may be deferred)

- gRPC channel reconnects after server restart
- RetryPolicy backoff respected
- Commands dropped or queued safely when CEF unavailable
- No blocking behavior under failure

## Exit Criteria

- Java process can issue gRPC commands without crashing
- Java survives CEF crashes
- No UI guarantees assumed

STOP: No CEF code, no JS, no UI semantics

# PHASE 6.2 — ~~JS ↔ IPC BRIDGE~~ CEF-side gRPC Endpoint (NO JS, NO UI LOGIC)

## Goal

~~Allow JS to send/receive IPC messages.~~
Expose a gRPC endpoint on the CEF process and connect it to the control plane without interacting with JS or UI behavior.

### Tests to Write

~~- JS can send READY~~
~~- NAVIGATE from Java works~~
~~- Error propagates back~~

- gRPC server starts inside CEF process
- Handshake RPC handled correctly
- NAVIGATE command received and queued
- CEF crash does not corrupt gRPC server state

### Implementation Rules

~~- Use CEF message router~~
~~- JSON only~~
~~- No business logic in JS~~

- gRPC runs on dedicated threads
- NO CEF UI calls from gRPC threads
- Commands marshalled to UI thread
- No JS bindings
- No renderer interaction

### Exit Criteria

~~- End-to-end flow working~~

- Java can invoke gRPC RPCs on CEF
- CEF accepts commands but does NOT execute UI actions yet
- No JS, no message router

STOP HERE — do NOT proceed to JS binding.

# PHASE 6.3 — JS Bridge 

## Goal 

- JS ↔ UI Bridge (Post-gRPC)

Bind received control commands to actual UI behavior via:
- JS bindings
- Message routing
- Browser / page control

This phase finally brings the UI in Working state

## Responsibilities

- JS ↔ native binding setup
- Renderer message routing
- Page navigation
- Contextual help routing
- Error propagation back to control plane (optional)

## Explicit Dependencies

- Phase 6.1 complete
- Phase 6.2 complete
- Stable .proto contract
- Stable threading boundaries
- Explicit Non-Goals
	- No transport changes
	- No retry policy changes
	- No lifecycle ownership changes
	- No security hardening yet

## Tests to Write

- JS handler invoked on command receipt
- Navigation triggered correctly
- Invalid commands handled gracefully
- Renderer crash does not crash CEF process

## Exit Criteria

- End-to-end command → UI behavior works
- UI restartable under control-plane supervision

STOP: No production hardening yet

# PHASE 7 — Hardening (FINAL) (gRPC-aware)

## Goal

Production readiness.

	- Tests
		- IPC drop recovery
		- Invalid messages
		- Java restart

	- Implementation		
		- Logging
		- Metrics hooks
		- Graceful shutdown

# Phase 7.1 — Security Hardening (gRPC-aware)

## Additions

- mTLS or secure channel configuration
- Token validation hardening
- Rate limiting at gRPC layer
- Input size limits
- Explicitly NOT Allowed
	- No protocol changes
	- No service signature changes

# Phase 7.2 — Resilience & Observability

## Additions

- gRPC health checks
- Structured logging for RPC lifecycle
- Metrics:
	- connection attempts
	- retry counts
	- command latency
	- Crash loop detection

# Phase 7.3 — Operational Readiness

## Additions

- Configurable ports
- Graceful shutdown ordering:
	- Stop accepting RPCs
	- Drain UI commands
	- Shutdown CEF
- Upgrade / backward compatibility strategy for .proto

## Exit Criteria (Phase 7)

- System survives:
	- crashes
	- restarts
	- network failures
- Fully observable
- Production deployable


# DEFINITION OF DONE

	- All tests pass
	- No cert warnings
	- Java controls UI
	- UI communicates state
	- Clean shutdown
