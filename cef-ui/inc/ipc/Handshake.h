#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include "MessageTypes.h"
#include "IMessageChannel.h"
#include "IpcProtocolException.h"

namespace cef_ui {
namespace ipc {

/// JSON message parser. 
/// Parses JSON strings into typed message objects.
/// 
/// Supported formats:
/// - HELLO: { "type": "HELLO", "sessionToken": "token_value" }
/// - NAVIGATE: { "type": "NAVIGATE", "url": "/path" }
///
/// Rules:
/// - Unknown fields are ignored
/// - Missing required fields cause rejection
/// - Malformed JSON throws IpcProtocolException
class MessageParser {
 public:
  MessageParser() = default;
  ~MessageParser() = default;

  /// Parse a JSON string into a typed message.
  /// @param json_string Raw JSON message string
  /// @return Parsed message (HelloMessageImpl or NavigateMessageImpl)
  /// @throws IpcProtocolException if JSON is malformed or required fields missing
  std::unique_ptr<Message> Parse(const std::string& json_string);

 private:
  MessageType GetMessageType(const std::string& type_string);
};

/// Handshake manager. Validates session tokens and manages handshake protocol.
///
/// Handshake flow:
/// 1. Client sends HELLO with sessionToken
/// 2. Server validates token
/// 3. If valid, handshake succeeds; if invalid, connection rejected
///
/// Token validation:
/// - Tokens are validated against an expected value
/// - Invalid tokens cause IpcProtocolException to be thrown
/// - Empty tokens are treated as invalid
class Handshake {
 public:
  /// Construct handshake validator with expected session token.
  /// @param expected_token The correct session token to validate against
  explicit Handshake(const std::string& expected_token);

  ~Handshake() = default;

  // Non-copyable
  Handshake(const Handshake&) = delete;
  Handshake& operator=(const Handshake&) = delete;

  /// Validate a HELLO message.
  /// @param message The parsed HelloMessageImpl to validate
  /// @throws IpcProtocolException if token is invalid or empty
  void ValidateHello(const HelloMessageImpl& message);

  /// Get the expected token (for testing).
  /// @return Expected session token
  const std::string& GetExpectedToken() const;

  /// Parse and validate a HELLO message from JSON.
  /// @param json_string The raw JSON message
  /// @throws IpcProtocolException if JSON is malformed or token invalid
  void ProcessHelloMessage(const std::string& json_string);

 private:
  std::string expected_token_;
  MessageParser parser_;
};

}  // namespace ipc
}  // namespace cef_ui
