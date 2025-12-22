#pragma once

#include <stdexcept>
#include <string>

namespace cef_ui {
namespace ipc {

/// Exception for IPC protocol violations.
/// Used to indicate errors in IPC messaging, handshake, and connection.
class IpcProtocolException : public std::runtime_error {
 public:
  explicit IpcProtocolException(const std::string& message)
      : std::runtime_error(message) {}

  explicit IpcProtocolException(const char* message)
      : std::runtime_error(message) {}
};

}  // namespace ipc
}  // namespace cef_ui
