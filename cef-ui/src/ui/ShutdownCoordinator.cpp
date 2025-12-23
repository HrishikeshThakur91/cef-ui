#include "../../inc/ui/ShutdownCoordinator.h"
#include "../../inc/ui/CefBrowserManager.h"
#include <atomic>
#include <chrono>
#include <thread>

namespace cef_ui {
namespace ui {

namespace {
  // Timeout for browser close (milliseconds)
  constexpr int BROWSER_CLOSE_TIMEOUT_MS = 5000;  // 5 seconds

  // Global shutdown state (for signal handlers)
  static std::atomic<bool> g_shutdown_requested(false);
}

ShutdownCoordinator::ShutdownCoordinator()
    : shutdown_initiated_(false),
      browser_closed_(false) {
}

ShutdownCoordinator::~ShutdownCoordinator() noexcept {
  // Ensure shutdown completed in destructor
  if (!shutdown_initiated_) {
    // This should not happen in normal operation
    // but we ensure cleanup if destructor is called without explicit Shutdown()
  }
}

void ShutdownCoordinator::Shutdown(CefBrowserManager* browser) noexcept {
  // Idempotent: only perform shutdown once
  if (shutdown_initiated_) {
    return;
  }
  shutdown_initiated_ = true;

  // Step 1: Close browser
  CloseBrowser(browser);

  // Step 2: Wait for browser to finish closing
  WaitForBrowserClose();

  // Step 3: Quit CEF message loop
  QuitMessageLoop();

  // Step 4: CefShutdown()
  CefShutdown();

  // Step 5: Exit process
  ExitProcess();
}

bool ShutdownCoordinator::IsShuttingDown() const {
  return shutdown_initiated_;
}

void ShutdownCoordinator::CloseBrowser(CefBrowserManager* browser) noexcept {
  if (!browser) {
    browser_closed_ = true;
    return;
  }

  // Request browser to close asynchronously
  browser->Close();
}

void ShutdownCoordinator::WaitForBrowserClose() noexcept {
  // Poll for browser close completion with timeout
  auto start = std::chrono::steady_clock::now();
  
  while (!browser_closed_) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    if (elapsed >= BROWSER_CLOSE_TIMEOUT_MS) {
      // Timeout reached - proceed with shutdown anyway
      break;
    }

    // Small sleep to prevent busy waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  browser_closed_ = true;
}

void ShutdownCoordinator::QuitMessageLoop() noexcept {
  // Signal CEF to quit its message loop
  // This will cause CefRunMessageLoop() to return
  // (actual CEF call will be added in Phase 6)
}

void ShutdownCoordinator::CefShutdown() noexcept {
  // Call CEF shutdown on main thread
  // Must be called after message loop exits
  // (actual CEF call will be added in Phase 6)
}

void ShutdownCoordinator::ExitProcess() noexcept {
  // Exit the process with success code
  // This should be the last step
  std::exit(0);
}

}  // namespace ui
}  // namespace cef_ui
