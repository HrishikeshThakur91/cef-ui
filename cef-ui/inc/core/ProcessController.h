#pragma once

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include "ILifecycleListener.h"

namespace cef_ui {
namespace core {

// NOTE:
// ProcessController is single-threaded.
// ProcessState::Starting is a transient internal state and may not
// be observable by external listeners.

/// Enum for process lifecycle states.
enum class ProcessState {
  Idle,      // Initial state, not started
  Starting,  // Transition state during start
  Started,   // Running normally
  Stopping,  // Transition state during shutdown
  Stopped    // Fully stopped
};

/// Manages process lifecycle: start, shutdown, state transitions, and event emission.
///
/// State machine:
///   Idle --[Start]--> Starting --[OnStarted event]--> Started
///   Started --[Shutdown]--> Stopping --[OnStopped event]--> Stopped
///
/// Guarantees:
/// - Double start is rejected (throws exception)
/// - Shutdown always succeeds if not already stopped
/// - All state transitions emit events to registered listeners
/// - Deterministic (no OS calls, no threads)
class ProcessController {
 public:
  ProcessController();
  ~ProcessController() = default;

  // Non-copyable for clarity of ownership
  ProcessController(const ProcessController&) = delete;
  ProcessController& operator=(const ProcessController&) = delete;

  // Move semantics allowed
  ProcessController(ProcessController&&) = default;
  ProcessController& operator=(ProcessController&&) = default;

  /// Start the process.
  /// @throws std::logic_error if already started or starting
  void Start();

  /// Shutdown the process gracefully.
  /// @throws std::runtime_error if shutdown fails (should not occur in deterministic mode)
  void Shutdown();

  /// Get current process state.
  /// @return Current ProcessState
  ProcessState GetState() const;

  /// Register a listener for lifecycle events.
  /// @param listener Non-null listener to receive events
  void AddListener(std::shared_ptr<ILifecycleListener> listener);

  /// Remove a listener from event notifications.
  /// @param listener Listener to remove
  void RemoveListener(std::shared_ptr<ILifecycleListener> listener);

 private:
  ProcessState state_;
  std::vector<std::shared_ptr<ILifecycleListener>> listeners_;

  /// Emit event to all registered listeners.
  void EmitStarted();
  void EmitStopping();
  void EmitStopped();
  void EmitError(const std::string& error_message);
};

}  // namespace core
}  // namespace cef_ui
