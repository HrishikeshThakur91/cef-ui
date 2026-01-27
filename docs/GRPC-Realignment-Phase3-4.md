# CEF Phase 3 & 4 Realignment: WSS ? gRPC

## Current State (WSS-based)

### Phase 3 — IPC CONTRACT (NO NETWORK YET)
- **Goal**: Define message schema & handshake logic without sockets
- **Message Schema**: JSON-based (HELLO, NAVIGATE)
- **Validation**: Session token verification via handshake
- **Assumptions**: Client-initiated connection, JSON messages, session-based auth

### Phase 4 — IPC TRANSPORT (WSS)
- **Goal**: Connect to Java WSS securely
- **Transport**: WebSocket Secure (TLS)
- **Pattern**: Client connects, sends HELLO, Java responds
- **Retry**: Exponential backoff on connection failure
- **Cleanup**: Graceful close on shutdown

---

## Required Changes for gRPC

### Why These Assumptions Break

1. **WSS is bidirectional client-initiated**: gRPC is **server-initiated (Java calls CEF)**
2. **JSON handshake**: gRPC uses **Protocol Buffers** (.proto files)
3. **Connection retry logic**: gRPC has **built-in service discovery & health checks**
4. **Session token in message**: gRPC uses **metadata & interceptors** for auth
5. **Single socket**: gRPC multiplexes **many streams** over HTTP/2

---

## Proposed Phase 3 Realignment (gRPC)

### Phase 3 — CONTROL-PLANE CONTRACT (NO NETWORK YET)

**Goal**: Understand CEF-side responsibilities in gRPC-based control plane.

**What CEF Must Know** (from .proto files):
- RPC service definition (methods Java will call)
- Message types (input/output contracts)
- Error codes and semantics
- Metadata requirements (authentication, tracing headers)

**What CEF Does NOT Know Yet**:
- How to start gRPC server (Phase 4)
- How to handle concurrent requests (Phase 6)
- Actual Java behavior or state

**Tests to Write** (integration layer only):
- Parse .proto file and validate CEF understands message types
- Validate error codes map to internal exceptions
- Validate metadata schema (headers, auth token placement)

**Implementation Rules**:
- **No network code**
- **No server setup**
- **No handler implementation**
- **No async/threading**
- Use .proto-generated stubs only (read-only)

**Exit Criteria**:
- CEF code compiles with gRPC-generated types
- No undefined message types
- All RPC signatures understood

---

## Proposed Phase 4 Realignment (gRPC)

### Phase 4 — GRPC TRANSPORT SETUP

**Goal**: Set up gRPC server skeleton on CEF side (Java calls in).

**What CEF Does**:
1. Define gRPC service stub (no handler logic)
2. Create gRPC server on fixed port (e.g., 50051)
3. Start server in dedicated thread
4. Register health checks
5. Graceful shutdown (drain requests, close)

**What CEF Does NOT Do** (saved for Phase 6):
- Implement actual RPC handlers (empty stubs only)
- Route messages to UI logic
- Modify browser state
- Return meaningful responses

**Threading Assumptions** (EXPLICIT):
- gRPC server runs in separate thread (not CEF message loop)
- RPC calls are synchronous from Java's perspective
- CEF message loop remains responsive to window events
- No blocking operations in RPC handlers (they're empty stubs)

**Tests to Write**:
- Server starts on specified port
- Server accepts connections and rejects without valid auth
- Health check responds correctly
- Server shuts down gracefully within timeout
- No CEF message loop blocked during RPC calls

**Implementation Rules**:
- **Use gRPC C++ API only**
- **No .proto implementation** (auto-generated code only)
- **No business logic in handlers**
- **No UI interaction**
- **No logging beyond startup/shutdown**
- **Single-threaded handlers** (synchronous, non-blocking)

**Exit Criteria**:
- gRPC server runs and accepts connections
- Health checks work
- RPC stubs are empty (no-op)
- Clean shutdown verified
- No crashes or hangs

---

## Mapping: Old Concepts ? New Concepts

| Old (WSS) | New (gRPC) | Notes |
|-----------|-----------|-------|
| Session token in HELLO message | gRPC metadata (auth header) | Handled by interceptors |
| Exponential backoff on connect fail | Built-in gRPC retries | Configured in .proto |
| Single persistent socket | HTTP/2 multiplexed streams | Automatic |
| JSON message validation | .proto-generated validation | Automatic |
| WssConnectionManager | GrpcServerManager (new) | Manages server lifecycle |
| Handshake logic | Interceptor chain (Phase 6) | Not in Phase 4 |
| Message routing | Handler stubs (Phase 6) | Not in Phase 4 |

---

## Files to Create (Design Only, No Code Yet)

### Phase 3 Resources
- `docs/grpc-contract.md` - gRPC service specification (reference .proto)
- `inc/grpc/GrpcContract.h` - Type definitions (auto-generated, read-only)

### Phase 4 Resources
- `inc/grpc/GrpcServerManager.h` - Server lifecycle wrapper
- `inc/grpc/GrpcHealthCheck.h` - Health check implementation
- `inc/grpc/IGrpcHandler.h` - Empty handler interface (will be populated in Phase 6)

### Code Organization
- **Phase 3**: Contracts live under `src/grpc/` (not `src/ipc/`)
- **Phase 4**: Server setup in `src/grpc/GrpcServerManager.cpp`
- **Phase 5**: Handler implementations in `src/grpc/handlers/` (Phase 6+)

---

## Explicit Threading Model (Phase 4)

```
Main Thread (CEF Message Loop)
??? CEF window events
??? Browser navigation
??? Browser JS callbacks

gRPC Thread
??? Server listens on 50051
??? Accepts incoming RPC calls
??? Queues responses
??? (Handlers are empty stubs - no blocking)

Synchronization:
- RPC handlers are synchronous (no async/await)
- No cross-thread calls yet (Phase 6)
- Each RPC response is independent
```

---

## Stop Conditions

**HARD STOP after Phase 4 realignment:**
- Do NOT implement actual RPC handlers
- Do NOT call CEF browser methods from gRPC thread
- Do NOT add message routing
- Do NOT add JS bindings

---

## Next Actions

1. ? **Complete**: Review existing TDD (this document)
2. ? **Pending**: Update `docs/cef-ui-tdd.md` with Phase 3 & 4 gRPC definitions
3. ? **Pending**: Design Phase 3 (contract understanding - stubs only)
4. ? **Pending**: Design Phase 4 (server setup - no handlers)
5. ? **Blocked**: Phase 6.2 implementation (wait for Phase 3 & 4 to be frozen)

