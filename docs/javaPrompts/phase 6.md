Role: You are implementing a Phase-6 Java-side MVP for a Java ↔ CEF system using gRPC as IPC transport.

### CONTEXT (DO NOT VIOLATE)
- Java 8
- gRPC Java 1.55.1
- Phase-6 only (functional integration)
- Unary gRPC only
- No TLS, no auth, no interceptors
- Java is authority, CEF is client
- Existing IPC interfaces already exist:
  IMessageChannel, IpcMessage, IpcServer, IpcConnection, Handshake,
  ConnectionListener, MessageTypes
- Do not redesign existing abstractions
- Phase 1–5 are complete and frozen
- This is Phase-6 only
- Java is the single authority
- CEF is a child process
- gRPC is used only as IPC transport
- Unary RPCs only
- Single CEF client
- Localhost only
- No TLS
- No authentication
- No interceptors
- No streaming
- No retries inside gRPC
- Placeholders for Phase-7 security are allowed only as interfaces / TODOs
    
### MVP GOAL - 

- Produce a working Java MVP that can:
    - Start a gRPC server
    - Accept one CEF client
    - Perform a handshake
    - Send an OPEN_PAGE command to CEF
    - Receive PAGE_STATUS notifications
    - Shut down cleanly
    - No UI. No VuePress logic yet.

### USE / ALIGN WITH EXISTING FILES - 

- You MUST align with these existing abstractions (do not redesign them):
    - IMessageChannel
    - IpcServer
    - IpcConnection
    - IpcMessage
    - MessageTypes
    - Handshake
    - ConnectionListener
Add new files only if necessary.

### REQUIRED OUTPUT (ONLY THESE)

Generate only:
- phase6.proto
    - Unary RPCs only
    - Commands:
        - Handshake
        - OpenPage
        - PageStatus
    - Include placeholders for metadata (Phase-7)
- GrpcIpcServer.java
    - Starts/stops gRPC server
    - Enforces single client
    - Binds to localhost
    - No TLS
- GrpcMessageChannel.java
    - Implements IMessageChannel
    - Converts proto ↔ IpcMessage
- Minimal wiring code to:
    - Start server
    - Accept handshake
    - Send one OPEN_PAGE command

### DO NOT GENERATE
- TLS config
- Auth logic
- Token validation
- VuePress server
- Process supervision
- Retry / backoff
- JS logic
- Metrics / logging frameworks
- Phase-7 features

### QUALITY RULES
- Code must compile
- Clear TODO comments for Phase-7 hooks
- No unused classes
- No speculative abstractions
- Keep code minimal and readable

### START WITH

- Step 1:
    - Generate phase6.proto only.
    - Wait for confirmation before generating Java classes.

--------------------------------------------------------------------------

You are implementing Phase-6 Java-side IPC using gRPC.

Context:
- Phase 1–5 are frozen
- Phase-6 only
- gRPC unary RPCs only
- No TLS, no auth, no interceptors
- Single CEF client
- Localhost only
- Java is authority

Task: Generate `GrpcIpcServer.java`.

Requirements:
- Start and stop a gRPC server
- Bind to localhost on a configurable port
- Register service generated from phase6.proto
- Enforce single-client connection
- Expose lifecycle methods:
  - start()
  - stop()
- No retry logic
- No logging framework
- No security logic
- Add TODO comments where Phase-7 TLS/auth will be added

Do NOT generate:
- Proto file
- Client code
- Business logic
- VuePress or UI code

Output:
Only `GrpcIpcServer.java`.

--------------------------------------------------------------------------

You are implementing Phase-6 gRPC service logic for Java IPC.

Context:
- gRPC unary RPCs only
- No TLS/auth
- Single CEF client
- Java is authority

Task: Generate `GrpcIpcService.java` (or similarly named class) that:

- Implements the gRPC service generated from phase6.proto
- Handles:
  - Handshake RPC
  - PageStatus RPC from CEF
- Validates:
  - protocolVersion
  - parentPid
- Notifies ConnectionListener on successful handshake
- Rejects duplicate handshake attempts

Rules:
- No retries
- No security checks
- No business logic
- No UI logic
- Keep logic minimal

Output:
Only the service implementation file.

----------------------------------------------------------------------------------

You are adapting gRPC transport to existing IPC abstractions.

Context:
- IMessageChannel already exists
- IpcMessage is the internal representation
- gRPC is transport only

Task: Generate `GrpcMessageChannel.java` that:

- Implements IMessageChannel
- Converts:
  - IpcMessage → gRPC request
  - gRPC request → IpcMessage
- Sends OPEN_PAGE command to CEF via gRPC
- Does NOT:
  - Own server lifecycle
  - Perform handshake validation
  - Perform retries
  - Cache messages

Rules:
- Keep mapping explicit
- No JSON parsing
- No speculative abstractions
- Add TODO hooks for Phase-7 metadata

Output:
Only `GrpcMessageChannel.java`.

--------------------------------------------------------------------------------------

You are wiring existing Handshake logic into Phase-6 gRPC flow.

Context:
- Handshake class already exists
- Handshake is protocol-level, not transport-level

Task:
Update Handshake usage so that:
- Handshake is triggered only once per CEF client
- Validation includes:
  - protocolVersion
  - parentPid
- Handshake failure terminates gRPC session

Rules:
- Do not change Handshake API
- Do not add security
- Do not add retries
- No transport logic inside Handshake

Output:
Only the modified or new wiring code.

--------------------------------------------------------------------------------------

You are creating a minimal Phase-6 Java MVP bootstrap.

Context:
- gRPC server exists
- IMessageChannel exists
- No UI, no VuePress yet

Task:
Generate a small bootstrap class that:

- Starts GrpcIpcServer
- Waits for handshake
- Sends one OPEN_PAGE command
- Prints PageStatus response
- Shuts down cleanly

Rules:
- No threading complexity
- No lifecycle supervision
- No retries
- No logging framework

Output:
One bootstrap class only.

--------------------------------------------------------------------------------------

Review the generated Java Phase-6 gRPC code and verify:

- No TLS or auth logic exists
- Unary RPCs only
- Single-client enforced
- Java remains authority
- No Phase-7 behavior implemented
- All TODOs clearly marked for Phase-7

Output:
A short checklist-style validation report.


------------------------------------------------------------------------------------------

# (TO DO)

✅ PROMPT 1 — Revisit & Realign Java Side Phases 3, 4, and 6  

Use this later, when you want to pause CEF work and re-align Java.

📌 Prompt: Java Control Plane Phase Realignment (gRPC)
We need to pause implementation and realign the Java control-plane design.

Context:
- We pivoted from IPC/WSS to gRPC.
- Phase 6.1 (Java runtime) was implemented early without full TDD alignment.
- This exposed misalignment with Java Phase 3 (contract) and Phase 4 (transport).
- CEF Phase 5 is stable and frozen.
- CEF Phase 6.2 has NOT started yet.

Goal of this session:
- Revisit and realign Java-side Phase 3, Phase 4, and Phase 6.1 ONLY.
- Ensure they form a clean, layered foundation for CEF Phase 6.2.
- Update the Java TDD accordingly.

Scope:
- Phase 3: gRPC contract definition (proto, semantics, ownership)
- Phase 4: gRPC transport lifecycle (channel, retry, reconnect)
- Phase 6.1: Java runtime usage of gRPC (no UI semantics)

Non-goals (STRICT):
- Do NOT implement new code.
- Do NOT touch CEF or native code.
- Do NOT add JS or UI logic.
- Do NOT proceed to Phase 6.2 or Phase 6.3.

Rules:
- Step-by-step only.
- No assumptions about CEF internals.
- Stop after updating the Java TDD and identifying required refactors.
- Explicit STOP line after realignment.

First step:
- Analyze current Java Phase 3, 4, and 6.1 responsibilities.
- Identify mismatches introduced by early gRPC runtime implementation.
- Propose corrected phase boundaries and contracts.

---------------------------------------------------------------------------------
Note - 
We need to pause implementation and realign Java-side design for Phases 3, 4, and 6 based on the finalized move to gRPC.

Context:

Java Phase 1–5 exist

Phase 6.1 (Java runtime gRPC control plane) was partially implemented without tests

Earlier Java TDD was written assuming IPC/WSS semantics

gRPC is now the authoritative transport

Goals for this session:

Revisit Java Phase 3 (Control Plane Contract) and redefine it as gRPC contract only

Revisit Java Phase 4 (Transport) and redefine it as gRPC runtime semantics

Re-evaluate Phase 6.1 implementation against the revised Phase 3 & 4

Identify mismatches, missing abstractions, and overreach

Propose TDD-aligned corrections, not code

Constraints:

NO CEF-side discussion

NO JS / UI logic

NO new features

NO implementation until alignment is complete

Changes must preserve phase boundaries

Deliverables:

Updated Phase 3 definition (Java, gRPC-only)

Updated Phase 4 definition (Java, gRPC runtime)

Corrected Phase 6.1 scope

Explicit STOP line after Phase 6.1

Do NOT generate code.
Work phase-by-phase, and stop after alignment.

-------------------------------------------------------------------------------
