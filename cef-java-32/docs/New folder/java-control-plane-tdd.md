## Global Rules for Java Control Plane TDD Implementation -

    - Do NOT auto-generate code for future phases
    - Do NOT scaffold multiple phases at once
    - Do NOT introduce placeholders for later phases
    - Java code must NOT reference CEF, browser, JS, or UI concepts directly
    - Java must treat the UI as an external, restartable subsystem

All code must be:
    
    - Deterministic
    - Thread-safe
    - Non-blocking
    - Testable without native/UI processes
    - Free of timing assumptions
    - Explicitly tolerant to UI unavailability

## JAVA CONTROL PLANE — TDD IMPLEMENTATION SPEC

    - Platform: Java 11+
    - Framework: Spring (core only)
    - Role: Authoritative Control Plane
    - UI: External native process (CEF-based)
    - IPC: Secure WebSocket (WSS)
    - Security: TLS + Session Token Handshake
    - Test Framework: JUnit 5 + Mockito
    - Approach: Phased TDD (NO SKIPS)

## GLOBAL RULES (MANDATORY)

    - Copilot / Developer MUST obey the following:
    - Tests first — always
    - One phase at a time
    - No JNI callbacks before Phase 6
    - No blocking waits on UI responses
    - No UI timing assumptions
    - Java MUST survive UI crash or restart
    - Java MUST NOT assume UI availability
    - All UI calls must be async / fire-and-forget
    - No business logic in UI-facing code
    - All tests must pass before next phase
    - No feature additions unless specified
    - Use dependency inversion for IPC & process control

# PROJECT STRUCTURE (INITIAL)

```
java-control-plane/
├── src/
│   ├── main/
│   │   └── java/
│   │       ├── config/
│   │       │   └── AppConfig.java
│   │       ├── lifecycle/
│   │       │   ├── ProcessController.java
│   │       │   └── LifecycleListener.java
│   │       ├── command/
│   │       │   ├── UiCommand.java
│   │       │   └── UiCommandType.java
│   │       ├── ipc/
│   │       │   ├── IMessageChannel.java
│   │       │   ├── MessageTypes.java
│   │       │   └── Handshake.java
│   │       └── transport/
│   │           └── (empty until Phase 4)
│   └── test/
│       └── java/
│           ├── config/
│           ├── lifecycle/
│           ├── command/
│           ├── ipc/
│           └── transport/
└── docs/
    └── java-control-plane-tdd.md
```

## PHASE 1 — Configuration & Contracts (NO IPC, NO UI)

### Goal

Establish immutable runtime configuration and command contracts.

### Write Tests FIRST

Create tests for:

    - AppConfig
        -Parses:
            - --ipcPort
            - --sessionToken
            - --startUrl
            - --windowId
        - Rejects missing arguments
        - Rejects invalid values

Expected behavior:

```
assertThrows(InvalidConfigException.class,
    () -> AppConfig.fromArgs(args));
```

InvalidConfigException:

    - Must extend RuntimeException
    - Must include a clear diagnostic message

Implementation Rules

    - No static globals
    - No singletons
    - Config object must be immutable

Exit Criteria

    - All config tests pass
    - No IPC code
    - No UI assumptions

## PHASE 2 — Process Lifecycle Controller (NO IPC, NO UI)

### Goal

Control lifecycle of an external UI process without knowing its internals.

### Tests to Write

ProcessController

    - Can start UI subsystem
    - Can shutdown UI subsystem
    - Rejects double-start
    - Can restart after crash
    - Emits lifecycle events:
        - onStarted
        - onStopping
        - onStopped
        - onError

Implementation Rules

    - No OS-level process management yet
    - No threads
    - Use mockable abstractions
    - Event-driven only

Exit Criteria

    - Deterministic lifecycle behavior
    - No IPC
    - No UI knowledge

## PHASE 3 — IPC CONTRACT (NO NETWORK)

### Goal

Define message schema and handshake logic without transport.

### Message Schema (LOCKED)

HELLO
```
{
  "type": "HELLO",
  "sessionToken": "abc"
}
```

OPEN
```
{
  "type": "OPEN",
  "windowId": "help"
}
```

NAVIGATE
```
{
  "type": "NAVIGATE",
  "contextId": "field_xyz"
}
```

### Tests to Write

    - Handshake accepts valid token
    - Handshake rejects invalid token
    - Unknown message type rejected
    - Missing required fields rejected
    - Extra fields ignored
    - JSON parsing errors handled gracefully

Implementation Rules

    - No sockets
    - No threads
    - Use interfaces only (IMessageChannel)
    - No reflection-based dispatch

Exit Criteria

    - IPC contract fully testable
    - No transport
    - No UI dependency

## PHASE 4 — IPC TRANSPORT (WSS)

### Goal

Securely connect to native UI process over WSS.

### Tests to Write

    - Can connect to WSS endpoint
    - TLS handshake succeeds
    - HELLO sent on connect
    - Reconnect on transient failure
    - Clean shutdown of transport

Implementation Rules

    - Use secure WebSocket implementation
    - Certificate validation via OS trust store
    - No insecure / bypass flags
    - No UI logic

Exit Criteria

    - IPC transport works with mock native peer
    - Secure by default
    - No UI coupling

## PHASE 5 — UI SUPERVISION (NO UI LOGIC)

### Goal

Supervise UI availability without embedding UI behavior.

### Tests to Write

    - Detect UI unavailable at startup
    - Retry UI start based on policy
    - Handle UI crash notification
    - Allow fire-and-forget UI commands even if UI is down
    - Preserve last known UI intent for replay

Implementation Rules
    
    - UI treated as external subsystem
    - No blocking waits
    - No command acknowledgements
    - No rendering or navigation logic

Exit Criteria

    - Java survives UI crashes
    - UI can be restarted safely
    - No assumptions about UI internals

## PHASE 6 — 

JS <-> IPC <-> CEF integration is NOT covered here.
Phase 6 is implemented only after CEF Phase 6 begins.

DEFINITION OF DONE (PHASE 1–5)

    - All tests pass
    - Java control plane stable
    - UI treated as restartable black box
    - No JNI, no JS, no browser assumptions
    - Ready for Phase 6 wiring
