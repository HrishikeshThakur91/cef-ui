// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_NATIVE_APPL_H_
#define CEF_NATIVE_APPL_H_

#include "include/cef_app.h"
#include <memory>
#include "../grpc/GrpcServer.h"
#include "../grpc/CommandQueue.h"


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

            // CefBrowserProcessHandler methods:
            void OnContextInitialized() override;
            CefRefPtr<CefClient> GetDefaultClient() override;

        private:
            // Process pending commands from gRPC (called on UI thread)
            void ProcessPendingCommands();
            // gRPC server for control plane communication
            std::unique_ptr<cef_ui::grpc_server::GrpcServer> grpc_server_;

            // Include the default reference counting implementation.
            IMPLEMENT_REFCOUNTING(CefNativeAppl);
        };

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
    }
}
