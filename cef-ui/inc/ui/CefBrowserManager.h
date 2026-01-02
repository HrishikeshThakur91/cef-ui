#pragma once

#include <string>
#include <memory>

namespace cef_ui {
namespace ui {

// Forward declarations
class NativeWindow;
//class CefClientHandler;
class BrowserInstance;

/// Single CEF browser instance manager.
/// 
/// Responsibilities:
/// - Create and bind single browser to native HWND
/// - Load HTTPS URL
/// - Manage browser lifecycle
/// 
/// Features:
/// - RAII-compliant
/// - Exception-safe
/// - Browser created while window is hidden
/// - Deterministic cleanup
/// 
/// Constraints:
/// - Single browser instance only
/// - No navigation logic
/// - No retries on load failure
/// - Non-copyable, non-movable
class CefBrowserManager {
 public:
  /// Create browser instance bound to window.
  /// 
  /// @param window Native window to bind browser to
  /// @param url HTTPS URL to load
  /// @throws std::runtime_error if browser creation fails
  CefBrowserManager(NativeWindow& window, const std::string& url);

  /// Destroy browser and release resources.
  /// Called automatically on destruction.
  ~CefBrowserManager() noexcept;

  // Non-copyable
  CefBrowserManager(const CefBrowserManager&) = delete;
  CefBrowserManager& operator=(const CefBrowserManager&) = delete;

  // Non-movable (browser is process resource)
  CefBrowserManager(CefBrowserManager&&) = delete;
  CefBrowserManager& operator=(CefBrowserManager&&) = delete;

  /// Check if browser is created and ready.
  /// @return true if browser exists and is not closing
  bool IsReady() const;

  /// Close the browser gracefully.
  /// Browser will close asynchronously.
  void Close() noexcept;

 private:
  NativeWindow& window_;
  std::string url_;
  ////std::unique_ptr<CefClientHandler> client_;
  //void* browser_;  // CefRefPtr<CefBrowser> (opaque to avoid CEF includes)
  //bool is_ready_;
  std::unique_ptr<BrowserInstance> browser_;

  /// Create browser instance with client handler.
  void CreateBrowser();

  /// Cleanup browser resources.
  void DestroyBrowser() noexcept;
};

}  // namespace ui
}  // namespace cef_ui
