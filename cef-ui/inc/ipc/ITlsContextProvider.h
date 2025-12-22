#pragma once

#include <string>
#include <memory>
#include "IWebSocketConnection.h"

namespace cef_ui {
namespace ipc {

/// Abstract TLS context provider for secure connections.
/// Encapsulates Windows native TLS (SChannel/SSPI) and OS certificate store.
/// No insecure bypass flags.
class ITlsContextProvider {
 public:
  virtual ~ITlsContextProvider() = default;

  /// Create and configure TLS context from OS store.
  /// @param host hostname for SNI and hostname verification
  /// @return configured TLS context (ownership transferred)
  /// @throws std::runtime_error if OS cert store unavailable
  virtual std::unique_ptr<ITlsContext> CreateTlsContext(
      const std::string& host) = 0;

  /// Check if TLS context creation is available.
  /// @return true if OS certs loaded and accessible
  virtual bool IsAvailable() const = 0;

  /// Get diagnostic info about TLS configuration.
  /// @return human-readable string with TLS/cert info
  virtual std::string GetDiagnostics() const = 0;
};

}  // namespace ipc
}  // namespace cef_ui
