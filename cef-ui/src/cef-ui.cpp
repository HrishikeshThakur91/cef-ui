#include <windows.h>

#include "include/cef_app.h"
#include "include/cef_browser_process_handler.h"

#include "../inc/ui/UIApplication.h"
#include "../inc/core/AppConfig.h"

using cef_ui::ui::UIApplication;

int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow) {
    //CefEnableHighDPISupport(); 

    void* sandbox_info = nullptr;

    CefMainArgs main_args(hInstance);

    int exit_code = CefExecuteProcess(main_args, nullptr, nullptr);
    if (exit_code >= 0)
        return exit_code;

    CefSettings settings;
    settings.no_sandbox = true;
    CefString(&settings.cache_path) =
        L"E:\\WorkSpace\\cef2\\cef-profile";

    if (!CefInitialize(main_args, settings, nullptr, nullptr))
        return -1;

    {
        // Your architecture starts here
        // Create config with required args (control channel optional)
        std::vector<std::string> args = {
            "--ipcPort", "8080",
            "--sessionToken", "dummy-token",
            "--startUrl", "https://www.google.com/",
            "--windowId", "0"
            // --controlFile and --controlKey are optional, not provided
        };
        cef_ui::core::AppConfig config = cef_ui::core::AppConfig::FromArgs(args);
        
        UIApplication app(config);
        app.Start();

        CefRunMessageLoop();
    }

    CefShutdown();
    return 0;
}


int main() {
    return wWinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineW(), SW_SHOW);
}