#include "../../inc/renderer/CefV8HandlerImpl.h"
#include "include/cef_v8.h"
#include "include/wrapper/cef_helpers.h"
#include <iostream>

namespace cef_ui {
namespace renderer {

bool CefV8HandlerImpl::Execute(const CefString& name,
                                CefRefPtr<CefV8Value> object,
                                const CefV8ValueList& arguments,
                                CefRefPtr<CefV8Value>& retval,
                                CefString& exception) {
    CEF_REQUIRE_RENDERER_THREAD();
    
    std::cout << "[CefV8Handler] JS called: " << name.ToString() << std::endl;
    
    // Get current context and frame
    CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
    if (!context) {
        exception = "No V8 context available";
        return true;
    }
    
    CefRefPtr<CefFrame> frame = context->GetFrame();
    if (!frame) {
        exception = "No frame available";
        return true;
    }
    
    // Handle openPage(url)
    if (name == "openPage") {
        // Validate arguments
        if (arguments.size() != 1) {
            exception = "openPage requires exactly 1 argument (url)";
            std::cerr << "[CefV8Handler] openPage rejected: wrong argument count" << std::endl;
            return true;
        }
        
        if (!arguments[0]->IsString()) {
            exception = "openPage argument must be a string";
            std::cerr << "[CefV8Handler] openPage rejected: argument not a string" << std::endl;
            return true;
        }
        
        std::string url = arguments[0]->GetStringValue().ToString();
        std::cout << "[CefV8Handler] openPage called with url: " << url << std::endl;
        
        // Create process message to send to browser process
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cef_control");
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        args->SetString(0, "openPage");
        args->SetString(1, url);
        
        // Send message to browser process
        frame->SendProcessMessage(PID_BROWSER, message);
        
        // Return success
        retval = CefV8Value::CreateBool(true);
        return true;
    }
    
    // Handle notifyReady()
    if (name == "notifyReady") {
        std::cout << "[CefV8Handler] notifyReady called" << std::endl;
        
        // Create process message to send to browser process
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("cef_control");
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        args->SetString(0, "notifyReady");
        
        // Send message to browser process
        frame->SendProcessMessage(PID_BROWSER, message);
        
        // Return success
        retval = CefV8Value::CreateBool(true);
        return true;
    }
    
    // Unknown function
    exception = "Unknown function: " + name.ToString();
    return true;
}

}  // namespace renderer
}  // namespace cef_ui
