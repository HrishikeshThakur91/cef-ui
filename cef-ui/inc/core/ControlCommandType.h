#pragma once

namespace cef_ui {
namespace core {

/// Enumeration of control command types.
/// Defines the types of commands that can be sent to the CEF process.
enum class ControlCommandType {
  Start,       ///< Start the CEF browser process
  Navigate,    ///< Navigate to a specific URL
  Shutdown,    ///< Shutdown the CEF browser process
  HealthPing   ///< Health ping to check process responsiveness
};

}  // namespace core
}  // namespace cef_ui
