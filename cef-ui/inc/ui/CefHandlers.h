#pragma once

#include <memory>

// Forward declarations for CEF types
namespace cef {
  class CefApp;
  class CefClient;
  class CefBrowser;
  class CefBrowserProcessHandler;
  class CefLifeSpanHandler;
  class CefLoadHandler;
}

namespace cef_ui {
namespace ui {

// Forward declarations
class NativeWindow;

/// Minimal CEF application handler.
/// 
/// Responsibilities:
/// - Provide browser process handler
/// - Minimal CEF initialization integration
/// 
/// Constraints:
/// - No message routing
/// - No JS bindings
/// - No IPC awareness
/// - Single instance per process
class CefAppHandler {
 public:
  /// Create CEF app handler.
  CefAppHandler();

  ~CefAppHandler();

  // Non-copyable
  CefAppHandler(const CefAppHandler&) = delete;
  CefAppHandler& operator=(const CefAppHandler&) = delete;

 private:
  // CEF app instance (will be initialized when CEF binaries available)
};

/// Minimal CEF client handler.
/// 
/// Responsibilities:
/// - Provide lifespan handler (window events)
/// - Provide load handler (page load events)
/// - Show window after successful page load
/// 
/// Constraints:
/// - No message routing
/// - No JS bindings
/// - No custom rendering
class CefClientHandler {
 public:
  /// Create CEF client handler with window reference.
  /// 
  /// @param window Native window to show after page load
  explicit CefClientHandler(NativeWindow* window);

  ~CefClientHandler();

  // Non-copyable
  CefClientHandler(const CefClientHandler&) = delete;
  CefClientHandler& operator=(const CefClientHandler&) = delete;

  /// Get lifespan handler for browser window events.
  /// @return Pointer to lifespan handler implementation
  class CefLifeSpanHandlerImpl* GetLifeSpanHandler();

  /// Get load handler for page load events.
  /// @return Pointer to load handler implementation
  class CefLoadHandlerImpl* GetLoadHandler();

 private:
  NativeWindow* window_;  // Borrowed reference (not owned)
  std::unique_ptr<class CefLifeSpanHandlerImpl> lifespan_handler_;
  std::unique_ptr<class CefLoadHandlerImpl> load_handler_;

  // Make handlers able to access window reference
  friend class CefLoadHandlerImpl;
};

}  // namespace ui
}  // namespace cef_ui
