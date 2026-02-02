// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_NATIVE_APPL_H_
#define CEF_NATIVE_APPL_H_

#include "include/cef_app.h"
#include <memory>

#include "../grpc/GrpcClient.h"


namespace cef_ui {
    namespace ui {

        // Implement application-level callbacks for the browser process.
        class CefNativeAppl : public CefApp, public CefBrowserProcessHandler {
        public:
            CefNativeAppl();
            ~CefNativeAppl();

            // CefApp methods:
            CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
                return this;
            }
            CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

            // CefBrowserProcessHandler methods:
            void OnContextInitialized() override;
            CefRefPtr<CefClient> GetDefaultClient() override;

            // Called by CefHandler when browser is created (Phase 6.3)
            void SetBrowser(CefRefPtr<CefBrowser> browser);

        private:
            // gRPC client for connecting to Java's server
            std::unique_ptr<cef_ui::grpc_client::GrpcClient> grpc_client_;

            // Browser instance for command execution
            CefRefPtr<CefBrowser> browser_;
            
            // Render process handler (Phase 6.3 Step 2)
            CefRefPtr<CefRenderProcessHandler> render_delegate_;

            // Include the default reference counting implementation.
            IMPLEMENT_REFCOUNTING(CefNativeAppl);
        };

    }  // namespace ui
}  // namespace cef_ui

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
