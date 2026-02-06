#pragma once

#include "ControlCommand.h"
#include "IControlCommandReceiver.h"
#include "../ui/BrowserInstance.h"
#include "../ui/ShutdownCoordinator.h"

namespace cef_ui {
namespace core {

/// Control command dispatcher for CEF UI.
/// 
/// Implements IControlCommandReceiver and dispatches commands to CEF UI components.
/// 
/// Command Handling:
/// - START: Log only (no-op)
/// - NAVIGATE: Load URL on main frame via BrowserInstance
/// - SHUTDOWN: Initiate graceful shutdown via ShutdownCoordinator
/// - HEALTH_PING: Log only (no-op)
/// 
/// Thread Safety:
/// - Methods may be called from any thread (e.g., file receiver thread)
/// - UI-affecting operations (browser_.LoadUrl) must be marshalled to the CEF UI thread
/// - Marshalling is the responsibility of the caller or integration layer
/// - This class does NOT perform thread marshalling internally
/// 
/// Constraints:
/// - No encryption
/// - No file IO
/// - No IPC
/// - UI logic only
/// - Single-window only
/// - No retry logic
/// - No health logic
/// - No crypto logic
class ControlCommandDispatcher : public IControlCommandReceiver {
 public:
  /// Creates a new ControlCommandDispatcher.
  /// @param browser Browser instance for navigation commands
  /// @param shutdown Shutdown coordinator for shutdown commands
  ControlCommandDispatcher(
      ui::BrowserInstance& browser,
      ui::ShutdownCoordinator& shutdown);

  ~ControlCommandDispatcher() override = default;

  // Non-copyable
  ControlCommandDispatcher(const ControlCommandDispatcher&) = delete;
  ControlCommandDispatcher& operator=(const ControlCommandDispatcher&) = delete;

  // IControlCommandReceiver implementation
  void OnCommand(const ControlCommand& command) override;
  void Shutdown() override;

 private:
  ui::BrowserInstance& browser_;
  ui::ShutdownCoordinator& shutdown_;
  bool shutdown_requested_ = false;  // Track shutdown idempotency

  /// Handle START command (log only).
  void HandleStart(const ControlCommand& command);

  /// Handle NAVIGATE command (load URL).
  void HandleNavigate(const ControlCommand& command);

  /// Handle SHUTDOWN command (initiate shutdown).
  void HandleShutdown(const ControlCommand& command);

  /// Handle HEALTH_PING command (log only).
  void HandleHealthPing(const ControlCommand& command);
};

}  // namespace core
}  // namespace cef_ui
