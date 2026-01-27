GLOBAL CONTEXT (DO NOT REINTERPRET):

- Goal: Minimal, resilient contextual help system.
- Architecture is FIXED and already agreed.
- C++ Phase 1–4 is complete and merged.
- Java Phase 1–4 is pending implementation.
- IPC model is request -> render -> response (monadic).
- No streaming, no chatty protocols, no generic platform building.
- CEF is a standalone UI process.
- Java is the authority and decision-maker.
- Scope must remain minimal unless explicitly expanded.

RULE:
- Do NOT add features, abstractions, or phases unless explicitly asked.



PHASE 1 – JAVA CONFIG & PROCESS INTEGRATION

CONTEXT:
- C++ Phase 1–4 IPC is complete and merged.
- Java application is authoritative for config and lifecycle.
- CEF will be launched as a separate process.

TASKS:
1. Define Java-side IPC configuration:
   - startUrl (must be https)
   - wssPort
   - sessionToken
   - windowId (optional)
2. Validate all config on startup:
   - reject invalid URLs
   - reject invalid ports
3. Expose a single method to build the CEF launch command.
4. Implement a lightweight ProcessController:
   - start CEF
   - detect unexpected exit
   - restart on next request

RULES:
- No IPC logic here
- No UI logic
- Fail fast on invalid config

PHASE 2 – JAVA IPC HANDSHAKE

CONTEXT:
- CEF connects via WSS.
- Session token validation must match C++ behavior exactly.

TASKS:
1. Implement HELLO message handling:
   - trim leading/trailing whitespace in sessionToken
   - compare against expected token
2. Reply with HELLO_ACK on success.
3. Reject invalid or malformed messages.

RULES:
- No retries here (client handles it)
- No bypass of validation
- Behavior must mirror C++ tests semantically


PHASE 3 – JAVA WSS ENDPOINT & ROUTING

CONTEXT:
- IPC is request/response, not streaming.
- Only a few message types are supported.

TASKS:
1. Create Spring HTTPS + WSS endpoint (localhost only).
2. Support message types:
   - SHOW_HELP
3. Route messages to handler methods.
4. Correlate requests using requestId.

RULES:
- Stateless per request
- No session registry
- No complex routing framework


PHASE 4 – JAVA TRANSPORT HARDENING

CONTEXT:
- Enterprise machines block insecure localhost traffic.
- TLS must rely on OS trust.

TASKS:
1. Configure HTTPS/WSS using OS-trusted certificate.
2. Enforce secure-only connections.
3. Gracefully close connections on shutdown.
4. Surface clear errors for invalid handshakes.

RULES:
- No custom crypto
- No insecure flags
- Keep configuration minimal
