# REQUEST TYPE: <Design Review | Implementation Guidance | Copilot Command | Debugging>

PHASE:

- Language: C++ | Java
- Phase Number: X
- Phase Name: ``` <as per cef-ui-tdd or agreed stories> ```

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

--------------------------------------------------------------------------------------

## Phase 6 Design & Implementation Planning

### Context (frozen):
- Phase 1–5 is COMPLETE and frozen on both Java and CEF (C++) sides.
- Phase 5 Java side provides lifecycle supervision only (no IPC, no command replay, no UI logic).
- Phase 5 CEF side owns window creation, browser lifecycle, renderer lifecycle.
- Phase 7 is explicitly reserved for security hardening, resilience, production readiness, observability, and ops.

🎯 First Goal of this chat - Design and plan Phase 6, whose responsibility is:
- All functional communication and IPC between Java and CEF, including application launch, command routing, page navigation, and crash-tolerant behavior.

### By the end of Phase 6:
- Java and CEF are fully functionally connected
- UI behavior is driven by Java
- CEF UI is restartable, redirectable, and resilient
- Phase 7 contains only hardening (no new behavior)

### 🚧 Hard boundaries (must not be violated)
- No Phase-7 concerns (security, auth, TLS hardening, production packaging)
- No premature optimization
- No reworking Phase 1–5 abstractions
- No mixing supervision logic back into Phase 6
- No “just for now” hacks
If something feels like Phase 7 → explicitly defer it.

### 🧩 Phase 6 responsibilities to design
#### 1️⃣ VuePress Server Lifecycle
- Decide who owns running the VuePress localhost server
	- Likely Java-side responsibility
	- Java should:
		- launch the server
		- monitor crashes
		- restart silently
- Clarify:
	- startup timing
	- health checks
	- restart policy
	- when CEF should attempt to load pages

#### 2️⃣ CEF Application Launch (Java → Native)
- Java must be able to launch the CEF application
- CEF window:
	- can be closed by user
	- can run independently in background
- Java must be able to:
	- detect if CEF is already running
	- bring it to foreground OR relaunch
- CEF exe path:
	- NOT finalized now
	- abstraction required
	- wiring later (Phase 7)

#### 3️⃣ IPC Initialization Strategy
- Define:
	- how IPC is established
	- who initiates handshake
	- message directionality
- Must support:
	- Java → CEF commands
	- CEF → Java error/status messages
- Must be restart-safe:
	- CEF crash → Java recovers
	- Java restart → CEF re-handshakes

#### 4️⃣ Core Use Cases to Support
* A. Documentation Launch (Button Click)
	- Existing Java button triggers:
		- Java launches CEF (if not running)
		- VuePress documentation opens in CEF
	- Replaces current PDF-based flow
* B. Contextual Help (Right-click → Show Help)
	- Java collects context data (JSON)
	- Java sends context payload to CEF
	- CEF determines:
		- exact VuePress page to open
	- Behavior rules:
		- If CEF not running → launch
		- If running → redirect page
		- If page not found → fallback page
	- On failure:
		- CEF must notify Java
		- User should NOT be disturbed

#### 5️⃣ Mapping & Routing Strategy
- VuePress repo must be parsed to build:
	- help-context → page mapping
- Decide:
	- where mapping lives (Java vs CEF)
	- when mapping is loaded
	- how it is updated
	- No manual hardcoding per page

#### Design questions to resolve in this chat
The assistant should:
- Ask clarifying questions before suggesting code
- Explicitly label:
	- Phase-6 decision
	- Deferred Phase-7 decision
- Produce:
	- Phase-6 architecture diagram (conceptual)
	- Clear responsibility split (Java vs CEF)
	- Step-by-step implementation plan
	- IPC message contract (initial version)

#### Expected outputs of this chat
By the end of this chat, we should have:
	- Final Phase-6 responsibility matrix
	- Decision on VuePress server ownership
	- Decision on CEF launch model
	- IPC handshake + message flow
	- Contextual help routing design
	- Clear “out of scope until Phase 7” list
	- Ordered implementation steps for Phase 6

## Important instruction to assistant:
- Stay disciplined.
- Ask before assuming.
- Phase-6 only.
- No drift.

--------------------------------------------------------------------------------
-----------------------------------------------------------------------------------

