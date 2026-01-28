#pragma once

#include "include/cef_v8.h"

namespace cef_ui {
namespace renderer {

/// V8 handler for JavaScript bindings
/// Handles calls from JS to native (window.cefControl.*)
class CefV8HandlerImpl : public CefV8Handler {
public:
    CefV8HandlerImpl() = default;
    
    /// Execute JavaScript function call
    /// @param name Function name (e.g., "openPage", "notifyReady")
    /// @param object The object the function is being called on
    /// @param arguments Function arguments from JavaScript
    /// @param retval Return value to JavaScript
    /// @param exception Exception message if call fails
    /// @return true if handled, false otherwise
    bool Execute(const CefString& name,
                 CefRefPtr<CefV8Value> object,
                 const CefV8ValueList& arguments,
                 CefRefPtr<CefV8Value>& retval,
                 CefString& exception) override;

private:
    IMPLEMENT_REFCOUNTING(CefV8HandlerImpl);
    DISALLOW_COPY_AND_ASSIGN(CefV8HandlerImpl);
};

}  // namespace renderer
}  // namespace cef_ui
