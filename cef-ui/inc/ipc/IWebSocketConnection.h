#pragma once

#include <string>
#include <memory>

namespace cef_ui {
namespace ipc {

/// TLS context interface for secure connections.
/// Encapsulates platform-specific TLS configuration.
class ITlsContext {
 public:
  virtual ~ITlsContext() = default;
};

/// Abstract WebSocket connection for sending/receiving frames.
/// Encapsulates low-level Asio/Beast details.
/// Primary test seam for Phase 4 unit tests.
class IWebSocketConnection {
 public:
  virtual ~IWebSocketConnection() = default;

  /// Establish connection to WSS endpoint.
  /// @param host hostname (e.g., "localhost" or "api.example.com")
  /// @param port port number (typically 443 for WSS)
  /// @param tls_context pre-configured TLS context
  /// @throws std::runtime_error if connection fails
  virtual void Connect(const std::string& host, uint16_t port,
                       const ITlsContext& tls_context) = 0;

  /// Send raw message frame.
  /// @param message message bytes to send
  /// @throws std::runtime_error if send fails or not connected
  virtual void Send(const std::string& message) = 0;

  /// Receive raw message frame (blocking).
  /// @return received message bytes
  /// @throws std::runtime_error if receive fails or timeout
  virtual std::string Receive() = 0;

  /// Check connection status.
  /// @return true if connected and ready for I/O
  virtual bool IsConnected() const = 0;

  /// Close connection gracefully.
  /// @throws std::runtime_error if close fails
  virtual void Close() = 0;

  /// Get last error message for diagnostics.
  /// @return human-readable error string
  virtual std::string GetLastError() const = 0;
};

}  // namespace ipc
}  // namespace cef_ui
