// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_HANDLER_H_
#define CEF_HANDLER_H_

#include <list>

#include "include/cef_client.h"
#include "../../cef-parallel/inc/ui/CefNativeAppl.h"

#include "CefUiBridgeImpl.h"


namespace cef_ui {
    namespace ui {

        class CefHandler : public CefClient,
            public CefDisplayHandler,
            public CefLifeSpanHandler,
            public CefLoadHandler {
        public:
            explicit CefHandler(bool is_alloy_style, CefUiBridgeImpl* bridge, CefNativeAppl* app_handler = nullptr);
            ~CefHandler() override;

            // Provide access to the single global instance of this object.
            static CefHandler* GetInstance();

            // CefClient methods:
            CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
            CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
            CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

            // CefDisplayHandler methods:
            void OnTitleChange(CefRefPtr<CefBrowser> browser,
                const CefString& title) override;

            // CefLifeSpanHandler methods:
            void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
            bool DoClose(CefRefPtr<CefBrowser> browser) override;
            void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

            // CefLoadHandler methods:
            void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                CefRefPtr<CefFrame> frame,
                int httpStatusCode) override;
            void OnLoadError(CefRefPtr<CefBrowser> browser,
                CefRefPtr<CefFrame> frame,
                ErrorCode errorCode,
                const CefString& errorText,
                const CefString& failedUrl) override;
            
            // CefClient methods:
            bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

            void ShowMainWindow();

            // Request that all existing browser windows close.
            void CloseAllBrowsers(bool force_close);

            bool IsClosing() const { return is_closing_; }

        private:
            // Platform-specific implementation.
            void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                const CefString& title);
            void PlatformShowWindow(CefRefPtr<CefBrowser> browser);

            // True if this client is Alloy style, otherwise Chrome style.
            const bool is_alloy_style_;

            // List of existing browser windows. Only accessed on the CEF UI thread.
            typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
            BrowserList browser_list_;

            bool is_closing_ = false;

            CefUiBridgeImpl* uiBridge_; // NOT owning
            CefNativeAppl* app_handler_; // NOT owning (Phase 6.3)

            // Include the default reference counting implementation.
            IMPLEMENT_REFCOUNTING(CefHandler);
        };

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
    }
}