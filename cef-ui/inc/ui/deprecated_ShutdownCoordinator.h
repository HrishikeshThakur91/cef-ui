#pragma once

#include <memory>

namespace cef_ui {
namespace ui {

// Forward declarations
class NativeWindow;
class CefBrowserManager;

/// Deterministic shutdown coordinator.
/// 
/// Responsibilities:
/// - Coordinate graceful shutdown sequence
/// - Handle WM_CLOSE from window
/// - Handle external shutdown requests
/// - Ensure proper CEF cleanup order
/// 
/// Shutdown Sequence:
/// 1. Close browser (asynchronous)
/// 2. Wait for browser close completion
/// 3. Quit CEF message loop
/// 4. Call CefShutdown()
/// 5. Exit process
/// 
/// Features:
/// - RAII-compliant
/// - Single shutdown sequence (idempotent)
/// - No force termination
/// - Exception-safe
/// 
/// Constraints:
/// - Single instance per process
/// - Must be called from main thread
/// - Non-copyable, non-movable
class ShutdownCoordinator {
 public:
  /// Create shutdown coordinator.
  ShutdownCoordinator();

  ~ShutdownCoordinator() noexcept;

  // Non-copyable
  ShutdownCoordinator(const ShutdownCoordinator&) = delete;
  ShutdownCoordinator& operator=(const ShutdownCoordinator&) = delete;

  // Non-movable
  ShutdownCoordinator(ShutdownCoordinator&&) = delete;
  ShutdownCoordinator& operator=(ShutdownCoordinator&&) = delete;

  /// Initiate graceful shutdown.
  /// Can be called multiple times safely (idempotent).
  /// 
  /// @param browser Browser to close (may be null)
  /// @throws std::runtime_error if shutdown sequence fails
  void RequestShutdown() noexcept;

  /// Check if shutdown is in progress.
  /// @return true if shutdown has been initiated
  bool IsShuttingDown() const;

 private:
  bool shutdown_initiated_;
  bool browser_closed_;

  /// Close the browser gracefully.
  void CloseBrowser(CefBrowserManager* browser) noexcept;

  /// Wait for browser to finish closing.
  void WaitForBrowserClose() noexcept;

  // /// Quit the CEF message loop.
  // void QuitMessageLoop() noexcept;

  // /// Call CefShutdown().
  // void CefShutdown() noexcept;

  // /// Exit the process.
  // void ExitProcess() noexcept;
};

}  // namespace ui
}  // namespace cef_ui
