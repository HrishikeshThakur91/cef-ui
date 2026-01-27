# Start C++ / CEF Side Phase 3 & 4 Realignment (Step-by-Step)

Use this when you’re ready to prepare CEF for gRPC, but before writing Phase 6.2 code.

📌 Prompt: CEF Side Phase 3 & 4 Realignment for gRPC -

-----------------------------------------------------------------------------------------------------------

We are starting CEF-side preparation for gRPC-based control plane integration.

Context:
- Phase 5 (CEF bootstrap, UI lifecycle) is COMPLETE and frozen.
- Java control plane will use gRPC.
- CEF Phase 6.2 (gRPC endpoint implementation) has NOT started.
- Existing CEF TDD was written assuming IPC/WSS.
- We must realign Phase 3 and Phase 4 on the CEF side BEFORE implementing Phase 6.2.

Goal of this session:
- Realign CEF Phase 3 and Phase 4 definitions to gRPC.
- Prepare a clean foundation for Phase 6.2 implementation.
- Update the CEF TDD accordingly.

Scope:
- Phase 3: Control-plane contract understanding on CEF side (proto consumption, semantics)
- Phase 4: gRPC transport setup on CEF side (server lifecycle, threading model)
- NO runtime UI behavior
- NO JS bindings
- NO renderer work

Non-goals (STRICT):
- Do NOT implement Phase 6.2 yet.
- Do NOT write production C++ code.
- Do NOT add gRPC handlers.
- Do NOT touch UI logic or browser navigation.

Rules:
- Step-by-step only.
- One phase at a time.
- Explicit STOP after Phase 4 realignment.
- All threading assumptions must be stated explicitly.
- No silent scope expansion.

First step:
- Review existing CEF Phase 3 & Phase 4 definitions in the TDD.
- Identify IPC/WSS assumptions that no longer apply.
- Propose gRPC-aligned replacements WITHOUT implementation.

-----------------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------------