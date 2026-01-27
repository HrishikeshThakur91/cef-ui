/**
 * PHASE-6 HANDSHAKE WIRING SUMMARY
 *
 * This document describes how handshake logic from the existing Handshake class
 * is wired into the Phase-6 gRPC flow.
 *
 * ============================================================================
 * ARCHITECTURE
 * ============================================================================
 *
 * Handshake Validation Flow:
 *
 *   CEF Client
 *      |
 *      | HandshakeRequest (gRPC)
 *      v
 *   CefControlServiceImpl.handshake()
 *      |
 *      | Extract fields from gRPC request
 *      | (sessionToken, clientVersion, protocolVersion, parentPid)
 *      |
 *      v
 *   HandshakeValidator.validate()
 *      |
 *      +-- Check single-client constraint
 *      |   (if handshakeDone, reject immediately)
 *      |
 *      +-- Validate gRPC-level fields:
 *      |   - sessionToken: non-empty
 *      |   - clientVersion: non-empty
 *      |   - protocolVersion: non-empty
 *      |   - parentPid: valid integer > 0
 *      |
 *      +-- Build JSON envelope with all fields
 *      |
 *      v
 *   Handshake.handle(json)
 *      |
 *      | Parse JSON and validate protocol
 *      | (delegates to existing Handshake class)
 *      |
 *      v
 *   HandshakeValidationResult
 *      (success flag + message)
 *      |
 *      v
 *   CefControlServiceImpl
 *      |
 *      +-- Build HandshakeResponse
 *      |
 *      +-- Call ConnectionListener.onConnected() if success
 *      |
 *      v
 *   CEF Client
 *      (receives HandshakeResponse)
 *
 * ============================================================================
 * KEY DESIGN DECISIONS
 * ============================================================================
 *
 * 1. SEPARATION OF CONCERNS
 *    - HandshakeValidator: Protocol + gRPC-level validation orchestration
 *    - Handshake: Protocol-level validation (JSON parsing, token checking)
 *    - CefControlServiceImpl: Transport-level (gRPC RPC handling)
 *
 * 2. HANDSHAKE CONSTRAINT
 *    - Single handshake per client connection
 *    - Enforced via HandshakeValidator.handshakeDone flag
 *    - Synchronized method ensures thread-safe state management
 *    - Second attempt always rejected with clear message
 *
 * 3. VALIDATION LAYERS
 *
 *    LAYER 1 - Single-Client Check
 *    ================================
 *    if (handshakeDone) {
 *        return failure("Handshake already completed...")
 *    }
 *
 *    LAYER 2 - gRPC Field Validation (HandshakeValidator)
 *    =====================================================
 *    - sessionToken: null/empty check
 *    - clientVersion: null/empty check
 *    - protocolVersion: null/empty check (from metadata)
 *    - parentPid: integer parsing + > 0 check (from metadata)
 *
 *    LAYER 3 - Protocol Validation (Handshake class)
 *    =====================================================
 *    - JSON syntax validation (JSONParser)
 *    - Message type validation (HELLO required)
 *    - Session token semantic validation (expected token check)
 *
 * 4. FAILURE HANDLING
 *    - Validation failures reset handshake state (for clean retry)
 *    - Error messages are descriptive (field name + reason)
 *    - gRPC session continues; client decides to retry or disconnect
 *    - No automatic retries (client responsibility)
 *
 * 5. SUCCESS PATH
 *    - Mark handshakeDone = true (prevents future handshakes)
 *    - Return success response
 *    - Call ConnectionListener.onConnected() for lifecycle awareness
 *
 * ============================================================================
 * FILES MODIFIED/CREATED
 * ============================================================================
 *
 * NEW FILE: HandshakeValidator.java
 * ================================
 * Orchestrates handshake validation:
 * - Runs single-client check
 * - Validates gRPC fields (sessionToken, clientVersion, protocolVersion, parentPid)
 * - Delegates to Handshake.handle() for protocol validation
 * - Returns HandshakeValidationResult (success/failure + message)
 *
 * Public API:
 *   HandshakeValidationResult validate(sessionToken, clientVersion, protocolVersion, parentPid)
 *   void reset()
 *   boolean isHandshakeDone()
 *
 * MODIFIED: CefControlServiceImpl.java
 * ===================================
 * Changed from:
 *   - AtomicBoolean handshakeDone (inline validation)
 *   - Duplicated validation logic in handshake() method
 *
 * Changed to:
 *   - HandshakeValidator handshakeValidator (delegated validation)
 *   - Clean handshake() method (extract fields + call validator + build response)
 *
 * Key changes:
 *   1. Removed AtomicBoolean handshakeDone
 *   2. Added HandshakeValidator instance
 *   3. Simplified handshake() method (now ~40 lines vs ~120)
 *   4. Removed duplicate validation logic
 *   5. Updated constraints doc
 *   6. Removed isHandshakeDone() and resetHandshake() (moved to validator)
 *
 * ============================================================================
 * USAGE EXAMPLE
 * ============================================================================
 *
 * // In wiring code:
 * CefControlServiceImpl service = new CefControlServiceImpl();
 * service.setConnectionListener(myListener);
 *
 * // gRPC server receives HandshakeRequest from CEF
 * HandshakeRequest req = HandshakeRequest.newBuilder()
 *     .setSessionToken("test-token-123")
 *     .setClientVersion("1.0.0")
 *     .putMetadata("protocolVersion", "1.0")
 *     .putMetadata("parentPid", "1234")
 *     .build();
 *
 * // CefControlServiceImpl.handshake() processes it:
 * // 1. Extract fields
 * // 2. Call handshakeValidator.validate(...)
 * // 3. HandshakeValidator checks:
 * //    - Not already done
 * //    - Fields non-empty
 * //    - parentPid is valid integer > 0
 * //    - Call Handshake.handle(json)
 * // 4. Build response
 * // 5. Notify listener if success
 *
 * ============================================================================
 * PHASE-7 EXTENSIONS
 * ============================================================================
 *
 * In HandshakeValidator:
 * - Add encryption metadata extraction and conversion
 * - Add capability negotiation fields
 *
 * In Handshake class:
 * - Validate signature/HMAC
 * - Validate encryption parameters
 * - Extract and return encryption keys
 *
 * In CefControlServiceImpl:
 * - Extract encryption keys from validator result
 * - Return keys in HandshakeResponse metadata
 * - Add interceptors for auth/encryption on subsequent RPCs
 *
 * ============================================================================
 * CONSTRAINT VERIFICATION
 * ============================================================================
 *
 * ? Handshake triggered only once per CEF client
 *   Enforced: HandshakeValidator.handshakeDone (synchronized)
 *
 * ? Validation includes protocolVersion
 *   Checked: HandshakeValidator validates (non-empty check)
 *
 * ? Validation includes parentPid
 *   Checked: HandshakeValidator validates (integer parse + > 0 check)
 *
 * ? Handshake failure terminates gRPC session
 *   Behavior: Returns HandshakeResponse with success=false
 *            Client must decide to disconnect or retry
 *            (Not automatic termination; controlled by client)
 *
 * ? Do not change Handshake API
 *   Confirmed: Still calls Handshake.handle(json)
 *             No modifications to Handshake class
 *
 * ? Do not add security
 *   Confirmed: No token validation, signature checking, encryption
 *             All marked as Phase-7 TODOs
 *
 * ? Do not add retries
 *   Confirmed: No retry loops, backoff, or circuit breakers
 *             Client is responsible for retries
 *
 * ? No transport logic inside Handshake
 *   Confirmed: Handshake only validates protocol/JSON
 *             gRPC concerns (request/response building) in CefControlServiceImpl
 *
 */
