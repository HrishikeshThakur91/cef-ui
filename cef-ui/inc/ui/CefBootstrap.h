#pragma once

namespace cef_ui {
namespace ui {

/// RAII-compliant CEF lifecycle for Phase 5, Step 1.
/// 
/// Responsibilities:
/// - Initialize CEF exactly once per process via CefInitialize()
/// - Provide Run() to enter CefRunMessageLoop()
/// - Call CefShutdown() AFTER message loop exits (not in destructor)
/// 
/// Thread Safety:
/// - Single-threaded
/// - Constructor blocks until CEF initialization completes
/// - Run() blocks until message loop exits
/// 
/// Exception Safety:
/// - Constructor throws if CefInitialize() fails
/// - Destructor is noexcept and does NOT call CefShutdown()
/// 
/// Lifecycle:
/// 1. Constructor calls CefInitialize() - throws on failure
/// 2. Run() calls CefRunMessageLoop() - blocks until exit
/// 3. Run() calls CefShutdown() AFTER message loop returns
/// 4. Destructor does nothing (CefShutdown already called)
/// 
/// Constraints:
/// - One instance per process (enforced by static flag)
/// - No window creation (Phase 5 Step 2+)
/// - No browser creation (Phase 5 Step 3+)
/// - No URL loading (Phase 5 Step 4+)
class CefBootstrap {
 public:
  /// Initialize CEF.
  /// Calls CefInitialize() with minimal settings.
  /// 
  /// @throws std::runtime_error if CefInitialize() fails
  CefBootstrap();

  /// Destructor does NOT call CefShutdown().
  /// CefShutdown() is called by Run() after message loop exits.
  ~CefBootstrap() noexcept;

  // Non-copyable (CEF is process-global)
  CefBootstrap(const CefBootstrap&) = delete;
  CefBootstrap& operator=(const CefBootstrap&) = delete;

  // Non-movable
  CefBootstrap(CefBootstrap&&) = delete;
  CefBootstrap& operator=(CefBootstrap&&) = delete;

  /// Enter CEF message loop and shutdown cleanly.
  /// 
  /// Sequence:
  /// 1. Calls CefRunMessageLoop() - blocks until all windows close
  /// 2. Calls CefShutdown() - cleans up CEF
  /// 3. Returns to caller
  /// 
  /// Must be called from the main thread after CEF is initialized.
  void Run();

 private:
  bool initialized_;

  /// Perform CefInitialize() with minimal settings.
  void Initialize();

  /// Perform CefShutdown() - called by Run() after message loop exits.
  void Shutdown() noexcept;
};

}  // namespace ui
}  // namespace cef_ui
