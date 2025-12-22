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

# PROJECT STRUCTURE (INITIAL)

cef-ui/
├── cef-ui/ <!--cef-ui Project-->
│	├── inc/
│   │	├── core/
│   │	|	├──	AppConfig.h
│   │	│	├──	ProcessController.h
│   │	|	└──	Logger.h
│   │	├── ipc/
│   │	|	├──	IMessageChannel.h
│   │	│	├──	MessageTypes.h
│   │	|	└─-	Handshake.h
│   │	└─- ui/
│   │		└─- (empty until Phase 4)
|	├──	src/
│   │	├── core/
│   │	|	├──	AppConfig.cpp
│   │	│	└─-	ProcessController.cpp
|	|	├── ipc/
│   │	├── ui/
│   |	└─- cef-ui.cpp <!-- main entry point -->
│	└──	CMakeLists.txt
├── cef-ui-tests/ <!--cef ui unit test Project-->
│   ├── inc/
│   │	├──	core/
│   │	├──	ipc/
│   │	└─-	ui/
│   └── src/
│      	├──	core/
│      	├──	ipc/
│      	├──	ui/
│		└──	test_main.cpp <!-- tests entry point -->
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

# PHASE 3 — IPC CONTRACT (NO NETWORK YET)

## Goal

Define message schema & handshake logic without sockets.

### Message Schema (LOCKED)

HELLO
{
  "type": "HELLO",
  "sessionToken": "abc"
}

NAVIGATE
{
  "type": "NAVIGATE",
  "url": "/docs/page"
}

### Tests to Write

	- Handshake accepts valid token
	- Rejects invalid token
	- Unknown message rejected
	- JSON parsing errors handled

### Implementation Rules

	- No sockets
	- No threads
	- Use interfaces only (IMessageChannel)

### Exit Criteria

	- IPC logic testable
	- No WebSocket
	- No CEF

Message parsing rules:

	- Unknown fields must be ignored
	- Missing required fields must reject the message
	- No dynamic casting

# PHASE 4 — IPC TRANSPORT (WSS)

## Goal

Connect to Java WSS securely.

### Tests to Write

	- Can connect to WSS endpoint
	- TLS handshake succeeds
	- HELLO sent on connect
	- Reconnect on failure

### Implementation Rules

	- Use secure WebSocket library
	- Certificate trust via OS store
	- No insecure flags

### Exit Criteria

	- IPC works against mock server
	- No UI yet

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

# PHASE 6 — JS ↔ IPC BRIDGE

## Goal

Allow JS to send/receive IPC messages.

### Tests to Write

	- JS can send READY
	- NAVIGATE from Java works
	- Error propagates back

### Implementation Rules

	- Use CEF message router
	- JSON only
	- No business logic in JS

### Exit Criteria

	- End-to-end flow working

# PHASE 7 — Hardening (FINAL)

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

# DEFINITION OF DONE

	- All tests pass
	- No cert warnings
	- Java controls UI
	- UI communicates state
	- Clean shutdown
