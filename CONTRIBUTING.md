# Contributing Guidelines

## Development Practices

### Configuration Parsing (AppConfig)
- Use two-phase model: parse all arguments first, validate after parsing completes
- Throw exceptions ONLY in validation phase (after iteration)
- Never throw during argument iteration
- Preserve existing validation rules for individual arguments during parsing

### Message Protocol (Handshake)
- Trim whitespace (space, tab, newline, carriage return) from RECEIVED session tokens ONLY
- Do NOT trim or modify the expected token
- Preserve internal whitespace in tokens
- Apply trimming immediately after token extraction, before comparison

### Testing & Mocking
- Use dependency injection for IPC components (IMessageChannel)
- Provide virtual test seams for system calls (e.g., Sleep)
- Mock external components rather than modifying production code with test logic
- Ensure unit tests execute in milliseconds (no blocking calls)

## Testing Standards

### Test Isolation
- All tests must be independent and deterministic
- Use mocks for external dependencies (WebSocket, timers)
- Create test support files in `cef-ui-tests/support/` for shared mock implementations
- Do not block on real system calls (Sleep, network I/O)

### Phase-Based Implementation
- Phases 1-4 are locked and production-ready
- Do not add features beyond specified phases
- All tests must pass before advancing phases