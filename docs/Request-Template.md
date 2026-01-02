# REQUEST TYPE: <Design Review | Implementation Guidance | Copilot Command | Debugging>

PHASE:
- Language: C++ | Java
- Phase Number: X
- Phase Name: <as per cef-ui-tdd or agreed stories>

REFERENCE FILES:
- <list of md / cpp / java files involved>

SCOPE (STRICT):
- Must implement only what is required for this phase.
- Must NOT introduce future-phase logic.
- Must reuse existing abstractions.
- Must assume other phases are unchanged.

TASK:
- <very specific outcome you want>

STOP CONDITIONS:
- State exactly when to stop and not continue.


----------------------------------------------------------------

# C++ PHASE DEFINITIONS (LOCKED):

	- Phase 1: Config & Process Control (DONE)
	- Phase 2: IPC Protocol & Handshake (DONE)
	- Phase 3: IPC Abstractions (DONE)
	- Phase 4: IPC Transport & Retry Policy (DONE)
	- Phase 5: CEF Standalone UI (minimal bootstrap + navigation)
	- Phase 6: Transport Hardening (optional / later)
	- Phase 7: Contextual Help Logic (mapping & fallback)


# JAVA PHASE DEFINITIONS (LOCKED):

	- Phase 1: Config & CEF Process Integration
	- Phase 2: IPC Protocol & Handshake
	- Phase 3: WSS Endpoint & Message Routing
	- Phase 4: Transport Hardening (TLS, lifecycle)
	- Phase 5+: Subtasks aligned with CEF phases

# A. Design Review Command

Review Phase <X> implementation against agreed minimal scope.

Do:
- Confirm alignment with requirements.
- Identify over-engineering or unnecessary constraints.

Do NOT:
- Suggest new features.
- Suggest refactors unless required for correctness.

Output:
- One of: 
	- Aligned 
	- Needs Simplification 
	- Misaligned
- Bullet list only.

# B. Copilot Command Generation

Generate a Copilot command list for:
- Language: <C++ | Java>
- Phase: <X>
- Scope: Minimal, phase-limited

Rules:
- Step-by-step.
- No future-phase placeholders.
- No assumptions beyond reference files.
- No changes to existing merged code.

Output:
- Markdown only.
- One command block per step.

# C. Implementation Guidance (Human-Readable)

Explain how to implement Phase <X> with minimal effort.

Constraints:
- Use existing abstractions.
- Prefer simplest working solution.
- Assume enterprise environment.

Output:
- High-level steps (numbered).
- No code unless explicitly asked.


# D. Debug / Failure Analysis

Analyse failing tests or behavior for Phase <X>.

Context:
- These failures are unexpected.

Rules:
- Do NOT redesign architecture.
- Identify exact cause.
- Propose minimal fix only.

Output:
- Root cause
- Minimal change
- Where to apply change

# EXPLICIT ANTI-DRIFT RULES (IMPORTANT)

ANTI-DRIFT CHECK:

- Is this required for the current phase? YES / NO
- Does this add new abstractions? YES / NO
- Does this assume future features? YES / NO

If any answer is YES -> stop and simplify.


# HOW TO USE MD FILES (cef-ui-tdd, java-md, etc.)

REFERENCE DOCUMENT:
- Name: cef-ui-tdd.md
- Authority: Source of truth
- Allowed actions:
  - Clarify interpretation
  - Validate alignment

Disallowed actions:
  - Rewrite phases
  - Add new requirements

