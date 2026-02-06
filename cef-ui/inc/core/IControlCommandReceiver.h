#pragma once

#include "ControlCommand.h"

namespace cef_ui {
namespace core {

/// Interface for receiving control commands.
/// Pure virtual interface for handling control commands sent to the CEF process.
/// Implementations will handle the actual command processing logic.
class IControlCommandReceiver {
 public:
  virtual ~IControlCommandReceiver() = default;

  /// Called when a control command is received.
  /// @param command The control command to process
  virtual void OnCommand(const ControlCommand& command) = 0;

  /// Shuts down this command receiver.
  /// Releases any resources associated with this receiver.
  /// After calling shutdown, no further commands should be processed.
  virtual void Shutdown() = 0;
};

}  // namespace core
}  // namespace cef_ui
