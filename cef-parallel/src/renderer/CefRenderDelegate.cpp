#include "../../inc/renderer/CefRenderDelegate.h"
#include "../../inc/renderer/CefV8HandlerImpl.h"
#include "include/wrapper/cef_helpers.h"
#include <iostream>

namespace cef_ui {
namespace renderer {

void CefRenderDelegate::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefV8Context> context) {
    CEF_REQUIRE_RENDERER_THREAD();
    
    std::cout << "[CefRenderDelegate] Creating JavaScript bindings" << std::endl;
    
    // Get the global object
    CefRefPtr<CefV8Value> global = context->GetGlobal();
    
    // Create the cefControl object
    CefRefPtr<CefV8Value> cefControl = CefV8Value::CreateObject(nullptr, nullptr);
    
    // Create the V8 handler
    CefRefPtr<CefV8Handler> handler = new CefV8HandlerImpl();
    
    // Create openPage function
    CefRefPtr<CefV8Value> openPageFunc = CefV8Value::CreateFunction("openPage", handler);
    cefControl->SetValue("openPage", openPageFunc, V8_PROPERTY_ATTRIBUTE_NONE);
    
    // Create notifyReady function
    CefRefPtr<CefV8Value> notifyReadyFunc = CefV8Value::CreateFunction("notifyReady", handler);
    cefControl->SetValue("notifyReady", notifyReadyFunc, V8_PROPERTY_ATTRIBUTE_NONE);
    
    // Attach cefControl to window
    global->SetValue("cefControl", cefControl, V8_PROPERTY_ATTRIBUTE_NONE);
    
    std::cout << "[CefRenderDelegate] JavaScript API registered: window.cefControl.openPage, window.cefControl.notifyReady" << std::endl;
}

}  // namespace renderer
}  // namespace cef_ui
