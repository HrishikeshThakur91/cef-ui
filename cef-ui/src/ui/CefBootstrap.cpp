#include "../../inc/ui/CefBootstrap.h"
#include <stdexcept>

// Include CEF headers
#include <include/cef_app.h>


namespace cef_ui {
    namespace ui {

        std::atomic<bool> CefBootstrap::process_initialized_{ false };

        CefBootstrap::CefBootstrap()
            : initialized_(false) {

            bool expected = false;
            if (!process_initialized_.compare_exchange_strong(expected, true)) {
                throw std::runtime_error("CEF can only be initialized once per process");
            }

            try {
                Initialize();
                initialized_ = true;
            }
            catch (...) {
                throw;
            }
        }

        CefBootstrap::~CefBootstrap() noexcept {
            // Do NOT call CefShutdown() here.
            // CefShutdown() is called by Run() after CefRunMessageLoop() exits.
            // Destructor is only a cleanup marker, CEF lifecycle is managed by Run().
        }

        void CefBootstrap::Run() {
            if (!initialized_) {
                throw std::runtime_error("CEF not initialized - call constructor first");
            }

            if (run_called_) {
                throw std::runtime_error("CefBootstrap::Run() may only be called once");
            }

            run_called_ = true;

            // Enter CEF message loop
            // This blocks until the application exits (e.g., all windows closed)
            CefRunMessageLoop();

            // CefRunMessageLoop() has returned - message loop is done
            // Now perform shutdown
            Shutdown();
        }

        void CefBootstrap::Initialize() {
            // Prepare main arguments (empty for Windows)
            CefMainArgs main_args;

            // Prepare minimal CEF settings
            CefSettings settings;
            // Use default settings for Phase 5 Step 1
            // Full configuration deferred to Phase 6+

            // Initialize CEF
            // app=nullptr (no CefApp handler in Phase 5 Step 1)
            // windows_sandbox_info=nullptr (Windows 7+ compatibility, not needed for basic init)
            if (!CefInitialize(main_args, settings, nullptr, nullptr)) {
                throw std::runtime_error("CefInitialize() failed");
            }
        }

        void CefBootstrap::Shutdown() noexcept {
            // Called by Run() after CefRunMessageLoop() exits
            // This is the proper place to call CefShutdown() per CEF documentation
            if (!initialized_) {
                return;
            }

            initialized_ = false;
            CefShutdown();
        }

    }  // namespace ui
}  // namespace cef_ui
