#pragma once

namespace cef_ui {
namespace core {

/// Interface for lifecycle event notifications.
/// Allows ProcessController to emit events to listeners without direct coupling.
class ILifecycleListener {
 public:
  virtual ~ILifecycleListener() = default;

  /// Called when the process transitions to Started state.
  virtual void OnStarted() = 0;

  /// Called when the process transitions to Stopping state.
  virtual void OnStopping() = 0;

  /// Called when the process transitions to Stopped state.
  virtual void OnStopped() = 0;

  /// Called when a fatal error occurs.
  /// @param error_message Human-readable error description
  virtual void OnError(const std::string& error_message) = 0;
};

}  // namespace core
}  // namespace cef_ui
