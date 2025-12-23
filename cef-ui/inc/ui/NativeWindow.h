#pragma once

#include <windows.h>
#include <string>
#include <map>

namespace cef_ui {
namespace ui {

/// Minimal native Win32 window for Phase 5, Step 2.
/// 
/// Responsibilities (PHASE 5, STEP 2 ONLY):
/// - Create a valid HWND
/// - Start in hidden state
/// - Keep message loop alive
/// - Close cleanly on user request
/// 
/// Features:
/// - RAII-compliant (window destroyed on object destruction)
/// - Exception-safe
/// - Deterministic cleanup
/// - Single window per instance
/// 
/// Constraints:
/// - No UI layout or controls
/// - No CEF browser creation (Phase 5 Step 3+)
/// - No URL loading (Phase 5 Step 3+)
/// - No message routing (Phase 6+)
/// - No shutdown coordination (Phase 5 Step 4+)
class NativeWindow {
 public:
  /// Create a hidden native window.
  /// 
  /// @param title Window title string (visible in taskbar)
  /// @throws std::runtime_error if window creation fails
  explicit NativeWindow(const std::string& title);

  /// Destroy window handle and release resources.
  /// Called automatically on destruction.
  ~NativeWindow() noexcept;

  // Non-copyable
  NativeWindow(const NativeWindow&) = delete;
  NativeWindow& operator=(const NativeWindow&) = delete;

  // Non-movable (HWND is process resource)
  NativeWindow(NativeWindow&&) = delete;
  NativeWindow& operator=(NativeWindow&&) = delete;

  /// Get the window handle.
  /// @return HWND of the native window
  HWND GetHandle() const;

  /// Static window procedure for Windows message handling.
  /// Public to allow registration with Windows API.
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

 private:
  HWND hwnd_;
  std::string title_;

  /// Initialize window class and create window.
  void InitializeWindow();

  /// Cleanup and destroy window.
  void DestroyWindowHandle() noexcept;

  /// Instance window procedure (dispatches to static proc).
  LRESULT OnMessage(UINT msg, WPARAM wparam, LPARAM lparam);
};

}  // namespace ui
}  // namespace cef_ui
