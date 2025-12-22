#pragma once

#include <string>
#include <memory>

namespace cef_ui {
namespace ipc {

/// Message type enumeration.
enum class MessageType {
  HELLO,      // Handshake initiation
  NAVIGATE,   // Navigation request
  UNKNOWN     // Unknown/invalid message type
};

/// Represents a parsed HELLO message.
/// Format: { "type": "HELLO", "sessionToken": "token_string" }
struct HelloMessage {
  std::string session_token;

  explicit HelloMessage(const std::string& token = "")
      : session_token(token) {}
};

/// Represents a parsed NAVIGATE message.
/// Format: { "type": "NAVIGATE", "url": "/path" }
struct NavigateMessage {
  std::string url;

  explicit NavigateMessage(const std::string& u = "")
      : url(u) {}
};

/// Base class for parsed messages.
class Message {
 public:
  virtual ~Message() = default;
  virtual MessageType GetType() const = 0;
};

/// Concrete HELLO message.
class HelloMessageImpl : public Message {
 public:
  explicit HelloMessageImpl(const std::string& session_token)
      : message_(session_token) {}

  MessageType GetType() const override {
    return MessageType::HELLO;
  }

  const std::string& GetSessionToken() const {
    return message_.session_token;
  }

 private:
  HelloMessage message_;
};

/// Concrete NAVIGATE message.
class NavigateMessageImpl : public Message {
 public:
  explicit NavigateMessageImpl(const std::string& url)
      : message_(url) {}

  MessageType GetType() const override {
    return MessageType::NAVIGATE;
  }

  const std::string& GetUrl() const {
    return message_.url;
  }

 private:
  NavigateMessage message_;
};

}  // namespace ipc
}  // namespace cef_ui
