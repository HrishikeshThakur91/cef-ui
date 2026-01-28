#pragma once

#include "include/cef_render_process_handler.h"

namespace cef_ui {
namespace renderer {

/// Render process handler for registering JavaScript bindings
class CefRenderDelegate : public CefRenderProcessHandler {
public:
    CefRenderDelegate() = default;
    
    /// Called when a V8 context is created
    /// This is where we register our JavaScript API
    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;

private:
    IMPLEMENT_REFCOUNTING(CefRenderDelegate);
    DISALLOW_COPY_AND_ASSIGN(CefRenderDelegate);
};

}  // namespace renderer
}  // namespace cef_ui
