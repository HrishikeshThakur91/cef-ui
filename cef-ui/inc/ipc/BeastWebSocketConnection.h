#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "IWebSocketConnection.h"

namespace cef_ui {
namespace ipc {

/// Production WebSocket connection using Boost.Beast and Windows SChannel TLS.
/// Implements IWebSocketConnection with synchronous I/O.
class BeastWebSocketConnection : public IWebSocketConnection {
 public:
  BeastWebSocketConnection();
  ~BeastWebSocketConnection() override;

  // Non-copyable
  BeastWebSocketConnection(const BeastWebSocketConnection&) = delete;
  BeastWebSocketConnection& operator=(const BeastWebSocketConnection&) = delete;

  /// Establish secure WebSocket connection.
  /// @param host hostname to connect to
  /// @param port port number (typically 443)
  /// @param tls_context TLS context configured by provider
  /// @throws IpcProtocolException if connection fails
  void Connect(const std::string& host, uint16_t port,
               const ITlsContext& tls_context) override;

  /// Send WebSocket text frame.
  /// @param message text message to send
  /// @throws IpcProtocolException if send fails
  void Send(const std::string& message) override;

  /// Receive WebSocket text frame (blocking).
  /// @return received text message
  /// @throws IpcProtocolException if receive fails
  std::string Receive() override;

  /// Check if WebSocket is connected.
  /// @return true if connected and ready for I/O
  bool IsConnected() const override;

  /// Close WebSocket connection gracefully.
  /// @throws IpcProtocolException if close fails
  void Close() override;

  /// Get last error message.
  /// @return diagnostic error string
  std::string GetLastError() const override;

 private:
  // Pimpl pattern to hide Boost dependencies
  class Impl;
  std::unique_ptr<Impl> impl_;
  
  bool connected_;
  std::string last_error_;
};

}  // namespace ipc
}  // namespace cef_ui