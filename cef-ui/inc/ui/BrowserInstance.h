#pragma once

#include <windows.h>
#include <string>
#include <memory>

namespace cef_ui {
namespace ui {

/// CEF browser instance wrapper for Phase 5, Step 4.
/// 
/// Responsibilities (PHASE 5, STEP 4 ONLY):
/// - Create a single CEF browser instance
/// - Bind browser to existing Win32 HWND
/// - Load HTTPS URL into browser
/// - Observe load callbacks (OnLoadStart, OnLoadEnd)
/// 
/// Assumptions:
/// - CEF is already initialized (CefBootstrap)
/// - A valid HWND already exists (NativeWindow)
/// - Message loop is already running (CefRunMessageLoop)
/// 
/// Features:
/// - RAII-compliant (browser destroyed on object destruction)
/// - Exception-safe
/// - Single browser instance only
/// - Minimal load handler for callback observation
/// 
/// Constraints:
/// - No window visibility control (Phase 5 Step 5+)
/// - No shutdown logic (Phase 5 Step 5+)
/// - No JS bindings (Phase 6+)
/// - No message routing (Phase 6+)
class BrowserInstance {
 public:
  /// Create a CEF browser bound to an existing HWND.
  /// 
  /// @param hwnd Valid Win32 window handle to bind browser to
  /// @throws std::runtime_error if browser creation fails
  explicit BrowserInstance(HWND hwnd);

  /// Destroy browser and release resources.
  /// Called automatically on destruction.
  ~BrowserInstance() noexcept;

  // Non-copyable
  BrowserInstance(const BrowserInstance&) = delete;
  BrowserInstance& operator=(const BrowserInstance&) = delete;

  // Non-movable (browser is CEF resource)
  BrowserInstance(BrowserInstance&&) = delete;
  BrowserInstance& operator=(BrowserInstance&&) = delete;

  /// Check if browser was created successfully.
  /// @return true if browser instance exists
  bool IsValid() const;

  /// Load HTTPS URL into the browser.
  /// Navigation happens asynchronously.
  /// 
  /// @param url HTTPS URL to load
  /// @throws std::runtime_error if URL is invalid or browser not ready
  void LoadUrl(const std::string& url);

 private:
  HWND hwnd_;
  void* browser_;  // Opaque CefRefPtr<CefBrowser> - holds actual browser
  void* load_handler_;  // Opaque CefRefPtr<CefLoadHandler> - observes load events

  /// Create browser instance and bind to HWND.
  void CreateBrowser();

  /// Cleanup browser resources.
  void DestroyBrowser() noexcept;

  /// Validate that URL is HTTPS.
  void ValidateHttpsUrl(const std::string& url) const;
};

}  // namespace ui
}  // namespace cef_ui
