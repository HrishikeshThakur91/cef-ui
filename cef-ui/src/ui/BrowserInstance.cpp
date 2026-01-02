#include "../../inc/ui/BrowserInstance.h"
#include <stdexcept>
#include <algorithm>

// Minimal CEF stubs for Phase 5 Step 4 - URL loading
// Will be replaced with actual CEF calls when CEF binaries are available
namespace cef {
  struct CefWindowInfo {
    void SetAsChild(void* hwnd) {
      // TODO: Link against actual libcef CefWindowInfo
    }
  };

  struct CefBrowserSettings {};

  struct CefBrowser {
    // Opaque CEF browser type
  };

  struct CefFrame {
    void LoadURL(const char* url) {
      // TODO: Link against actual libcef CefFrame::LoadURL
    }
  };

  struct CefLoadHandler {
    virtual ~CefLoadHandler() = default;
    virtual void OnLoadStart(CefBrowser* browser, CefFrame* frame) {}
    virtual void OnLoadEnd(CefBrowser* browser, CefFrame* frame, int http_status_code) {}
    virtual void OnLoadError(CefBrowser* browser, CefFrame* frame, int error_code,
                             const char* error_text, const char* failed_url) {}
  };

  // Minimal load handler implementation for Phase 5 Step 4
  class MinimalLoadHandler : public CefLoadHandler {
   public:
    void OnLoadStart(CefBrowser* browser, CefFrame* frame) override {
      // Load started - callback observed
    }

    void OnLoadEnd(CefBrowser* browser, CefFrame* frame, int http_status_code) override {
      // Load completed - callback observed
    }

    void OnLoadError(CefBrowser* browser, CefFrame* frame, int error_code,
                     const char* error_text, const char* failed_url) override {
      // Load failed - callback observed
    }
  };

  class CefBrowserHost {
   public:
    static void* CreateBrowser(CefWindowInfo& window_info,
                               CefBrowserSettings& settings,
                               const char* url,
                               void* client,
                               void* extra_info) {
      // TODO: Link against actual libcef CefBrowserHost::CreateBrowser
      // For Phase 5 Step 3-4, just return non-null to indicate success
      return new CefBrowser();
    }

    static CefFrame* GetMainFrame(void* /*browser*/) {
        static CefFrame frame;
        return &frame;
    }

    static void CloseBrowser(void* browser, bool force_close) {
      // TODO: Link against actual libcef
    }
  };
}

namespace cef_ui {
    namespace ui {

        BrowserInstance::BrowserInstance(HWND hwnd)
            : hwnd_(hwnd),
            browser_(nullptr),
            load_handler_(nullptr) {

            if (!hwnd) {
                throw std::runtime_error("HWND cannot be null");
            }

            try {
                CreateBrowser();
            }
            catch (...) {
                DestroyBrowser();
                throw;
            }
        }

        BrowserInstance::~BrowserInstance() noexcept {
            DestroyBrowser();
        }

        bool BrowserInstance::IsValid() const {
            return browser_ != nullptr;
        }

        void BrowserInstance::LoadUrl(const std::string& url) {
            if (shutdown_initiated_) {
                throw std::runtime_error("BrowserInstance is shut down");
            }
            if (!browser_) {
                throw std::runtime_error("Browser not created");
            }

            // Validate HTTPS URL
            ValidateHttpsUrl(url);

            // Get main frame and load URL
            cef::CefFrame* frame = cef::CefBrowserHost::GetMainFrame(browser_);
            if (!frame) {
                throw std::runtime_error("Failed to get browser main frame");
            }

            // Navigate to URL (asynchronous)
            frame->LoadURL(url.c_str());
        }

        void BrowserInstance::CreateBrowser() {
            // Create browser window info
            cef::CefWindowInfo window_info;

            // Bind browser to existing Win32 HWND
            window_info.SetAsChild(hwnd_);

            // Use default browser settings (minimal)
            cef::CefBrowserSettings settings;

            // Create minimal load handler for callbacks
            load_handler_ = new cef::MinimalLoadHandler();

            // Create browser instance
            // Empty URL for Step 3 - URL loaded in Step 4 via LoadUrl()
            browser_ = cef::CefBrowserHost::CreateBrowser(
                window_info,
                settings,
                "",          // Empty URL - will be loaded via LoadUrl() in Phase 5 Step 4
                load_handler_,  // Minimal load handler for callback observation
                nullptr      // No extra info
            );

            if (!browser_) {
                throw std::runtime_error("Failed to create CEF browser instance");
            }
        }

        void BrowserInstance::DestroyBrowser() noexcept {
            if (shutdown_initiated_) {
                return;
            }

            shutdown_initiated_ = true;

            // Phase 5 deterministic shutdown:
            // We explicitly close and delete owned resources.
            if (browser_) {
                // Phase 5 stub: explicit close point
                // Phase 6+: replace with CefBrowserHost::CloseBrowser(...)
                delete static_cast<cef::CefBrowser*>(browser_);
                browser_ = nullptr;
            }

            if (load_handler_) {
                delete static_cast<cef::MinimalLoadHandler*>(load_handler_);
                load_handler_ = nullptr;
            }
        }

        void BrowserInstance::ValidateHttpsUrl(const std::string& url) const {
            if (url.empty()) {
                throw std::runtime_error("URL cannot be empty");
            }

            // Check that URL starts with https://
            const std::string https_prefix = "https://";
            if (url.compare(0, https_prefix.length(), https_prefix) != 0) {
                throw std::runtime_error("Only HTTPS URLs are allowed");
            }
        }

    }  // namespace ui
}  // namespace cef_ui
