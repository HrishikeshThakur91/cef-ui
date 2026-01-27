Task: Implement Phase 1 ONLY of Java Control Plane TDD.

Rules (mandatory):
- Tests FIRST (JUnit 5).
- Implement ONLY what tests require.
- Do NOT reference CEF, browser, UI, IPC, sockets, processes, threads, timers, or native concepts.
- No placeholders for future phases.
- No static globals, no singletons.

Scope: Implement immutable runtime configuration parsing.

Classes allowed:
- config.AppConfig
- config.InvalidConfigException

Behavior to test and implement:
- AppConfig.fromArgs(String[] args)
	- Requires arguments:
		- --ipcPort (int, 1024–65535)
		- --sessionToken (non-empty string)
		- --startUrl (non-empty string)
		- --windowId (non-empty string)
	- Rejects:
		- Missing arguments
		- Unknown flags
		- Invalid numeric ranges
		- Empty values

- InvalidConfigException
	- Extends RuntimeException
	- Message must clearly identify the failing parameter

Constraints:
- AppConfig must be immutable
- No setters
- No environment variable access
- No defaults

Exit condition:
- All config tests pass
- No other packages touched

-----------------------------------------------------------------------

Task: Implement Phase 2 ONLY of Java Control Plane TDD.

Rules (mandatory):
- Tests FIRST
- No threads
- No timers
- No ProcessBuilder, Runtime.exec, or OS calls
- No IPC, sockets, JNI, UI, browser, or CEF references

Scope: External subsystem lifecycle modeling ONLY.

Classes allowed:
- lifecycle.ProcessController
- lifecycle.LifecycleListener

Behavior to test and implement:
- start()
	- Emits onStarted() exactly once
	- Rejects double-start
- stop()
	- Emits onStopping() then onStopped()
- restart()
	- Allowed only after stopped or error
- onError(Throwable)
	- Transitions to stopped state

Constraints:
- State machine must be explicit
- Events must be deterministic
- Listener invocation order must be testable

Exit condition:
- All lifecycle tests pass
- No process management code exists

-----------------------------------------------------------------------

Task: Implement Phase 3 ONLY of Java Control Plane TDD.

Rules:
- Tests FIRST
- No sockets
- No WebSocket libs
- No threads
- No reflection-based dispatch

Scope: Message schema + validation only.

Classes allowed:
- ipc.IMessageChannel (interface only)
- ipc.Handshake
- ipc.MessageTypes

Behavior to test and implement:
- Accept valid HELLO with correct sessionToken
- Reject invalid token
- Reject unknown message types
- Reject missing required fields
- Ignore extra JSON fields
- Handle malformed JSON without throwing

Constraints:
- Deterministic parsing
- No side effects

Exit condition:
- All IPC contract tests pass
- No transport code exists

-----------------------------------------------------------------------

PHASE 4 — COPILOT PROMPT

Task: Implement Phase 4 ONLY of Java Control Plane TDD.

Rules:
- Tests FIRST
- Secure WSS only
- No insecure flags
- No UI logic

Scope: Transport implementation behind interfaces.

Behavior to test:
- Connect to WSS endpoint
- TLS handshake succeeds using OS trust store
- HELLO sent immediately on connect
- Reconnect on transient failure
- Clean shutdown

Exit condition:
- Mock native peer tests pass

-----------------------------------------------------------------------

PHASE 5 — COPILOT PROMPT

Task: Implement Phase 5 ONLY of Java Control Plane TDD.

Rules:
- Tests FIRST
- No UI logic
- No navigation concepts
- No acknowledgements

Scope: Supervisory logic only.

Behavior to test:
- Detect UI unavailable at startup
- Retry UI start via policy
- Handle UI crash notifications
- Allow fire-and-forget commands even when UI is down
- Preserve last intent for replay after restart

Constraints:
- Non-blocking
- Deterministic
- UI treated as black box

Exit condition:
- Java survives UI crashes
- No JNI, no JS, no browser assumptions

-----------------------------------------------------------------------

# java-phase6-integration-tdd.md

## 1. Purpose

This document defines the **Java-side Technical Design** for **Phase-6 (Functional Integration)** of the Java ↔ CEF system.

### Scope
- Phase-6 ONLY
- Functional wiring and orchestration
- Activation of existing Phase-1–5 abstractions

### Explicit Non-Goals
- No security
- No hardening
- No production readiness
- No redesign of earlier phases

Phase-7 will exclusively handle:
- TLS / WSS
- Authentication / tokens
- Signing / checksums
- Observability / metrics
- Packaging / distribution
- Single-instance enforcement

---

## 2. Frozen Inputs (Authoritative)

The following are **immutable and MUST NOT be modified**:
- Phase-1–5 abstractions
- Process model decisions
- IPC envelope format
- Authority boundaries

Any change request affecting these is **out of scope**.

---

## 3. Authority Model

### Java (Control Plane)
Java is the **single authority** for:
- UI behavior
- Navigation
- Context resolution
- Process lifecycle
- Recovery decisions
- IPC server lifecycle
- VuePress server lifecycle

### CEF
- Render-only UI process
- Executes commands issued by Java
- Owns no business logic
- Owns no navigation logic

### JavaScript (CEF)
- Render-only
- No business decisions
- No navigation control
- No IPC command authority

---

## 4. Process Model

### Architecture
- Java and CEF run as **separate OS processes**
- CEF is always spawned as a **child process of Java**

### Lifecycle Rules
| Event | Behavior |
|---|---|
| Java exits | CEF exits immediately |
| User closes CEF | CEF exits; Java does nothing |
| CEF crashes | Java relaunches CEF |
| Java restarts | CEF must exit |

### Constraints
- CEF relaunch occurs **only** on explicit Java logic
- Multiple CEF instances are prevented by **convention only**
- No OS locks or mutexes (Phase-7 concern)

---

## 5. VuePress Documentation Hosting

### Content Model
- VuePress content is **prebuilt static files**
- No runtime VuePress build
- No dynamic content generation

### Java Responsibilities
- Host content via embedded HTTP server
- Bind server to `127.0.0.1`
- Use an **ephemeral port**
- Restart server on crash
- Notify CEF before and after restart

### URL Contract

http://127.0.0.1:<port>/


### Restart Semantics
1. Server crash detected
2. Java restarts server
3. Java sends `DOCS_RESTARTING`
4. Java sends `DOCS_READY` with new URL

No HTTPS. No caching or compression tuning.

---

## 6. IPC Transport (Phase-6)

### Transport
- Local TCP
- Bound to `127.0.0.1`
- Port selected by Java
- Passed to CEF via argv
- Single CEF client only

### Communication Model
- Request → Response
- Explicit command correlation
- No streaming
- No multiplexing

---

## 7. IPC Protocol

### Mandatory JSON Envelope
```json
{
  "protocolVersion": "1.0",
  "commandId": "uuid",
  "command": "COMMAND_NAME",
  "headers": {},
  "payload": {}
}


---

### ✅ What You Have Now
- A **final Java Phase-6 TDD**
- A **hard compliance checklist** suitable for PR reviews
- A **Phase-7 firewall** baked into the document

---

### Next Logical Steps (Your Call)
1. Generate **CEF-side Phase-6 TDD** to match this
2. Generate **Java package-level TODO map** referencing this TDD
3. Start **Java class-by-class implementation walkthrough**

Tell me what you want next — we stay locked and disciplined.


-----------------------------------------------------------------------
						Commit Messages 
-----------------------------------------------------------------------
✅ Phase 1 — Configuration & Contracts

Commit message

Phase 1: Add immutable runtime configuration and validation (Java control plane)

- Introduced AppConfig with strict argument parsing
- Added InvalidConfigException with clear diagnostics
- Enforced immutability and deterministic behavior
- Covered all validation paths with unit tests
- No IPC, UI, lifecycle, or native assumptions


Why this is good

Signals “foundation only”

Explicitly says what is not included

Safe to cherry-pick or revert independently

✅ Phase 2 — Process Lifecycle Controller

Commit message

Phase 2: Implement deterministic UI subsystem lifecycle controller

- Added ProcessController with explicit state machine
- Defined LifecycleListener event contract
- Enforced start/stop/restart invariants
- Covered error and double-start scenarios via tests
- No OS process control, IPC, threads, or UI knowledge


Why this is good

Emphasizes “modeling”, not execution

Makes it clear native lifecycle is untouched

✅ Phase 3 — IPC Contract (No Transport)

Commit message

Phase 3: Define IPC message contract and handshake validation

- Added message type definitions and schema validation
- Implemented handshake logic with session token verification
- Rejected unknown, malformed, or invalid messages safely
- Ignored extra fields by design
- No network, sockets, transport, or UI coupling


Why this is good

Locks protocol without implying connectivity

Makes Phase 4 review much easier

✅ Phase 4 — Secure IPC Transport (WSS)

Commit message

Phase 4: Add secure WSS transport for control-plane IPC

- Implemented WebSocket transport behind IMessageChannel
- Enforced TLS via OS trust store
- Sent HELLO handshake on successful connect
- Added reconnect and clean shutdown handling
- No UI logic or native assumptions


Why this is good

Clearly states security posture

Separates transport from behavior

✅ Phase 5 — UI Supervision (Phase 5 FINAL / FROZEN)

Commit message

Phase 5: Add UI supervision and resilience (CEF-agnostic)

- Treated UI as external, restartable subsystem
- Allowed fire-and-forget commands during UI downtime
- Preserved last intent for replay after restart
- Handled UI crash and unavailability deterministically
- No UI logic, navigation, JNI, or JS assumptions


Why this is good

Explicitly calls out CEF-agnostic

Makes it very clear Phase 6 is where wiring begins

🧊 Optional Meta Commit (Recommended)

After Phase 5 is frozen:

Freeze Phase 1–5: Java control plane stable and ready for Phase 6

- All Phase 1–5 tests passing
- No native, JNI, JS, or browser coupling
- UI treated as black-box external subsystem
- Ready for IPC ↔ JS ↔ CEF wiring in Phase 6