// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#pragma once

#include <string>
#include <iostream>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_callback.h"

#include "../../cef-parallel/inc/ui/CefHandler.h"
#include "../../cef-parallel/inc/grpc/UiCommand.h"  
#include "../../cef-parallel/inc/ui/CefNativeAppl.h"
#include "../../cef-parallel/inc/renderer/CefRenderDelegate.h"


namespace cef_ui {
    namespace ui {

        // When using the Views framework this object provides the delegate
        // implementation for the CefWindow that hosts the Views-based browser.
        class CefWinDelegate : public CefWindowDelegate {
        public:
            CefWinDelegate(CefRefPtr<CefBrowserView> browser_view,
                cef_runtime_style_t runtime_style,
                cef_show_state_t initial_show_state)
                : browser_view_(browser_view),
                runtime_style_(runtime_style),
                initial_show_state_(initial_show_state) {
            }

            void OnWindowCreated(CefRefPtr<CefWindow> window) override {
                // Add the browser view and show the window.
                window->AddChildView(browser_view_);

                if (initial_show_state_ != CEF_SHOW_STATE_HIDDEN) {
                    window->Show();
                }
            }

            void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
                browser_view_ = nullptr;
            }

            bool CanClose(CefRefPtr<CefWindow> window) override {
                // Allow the window to close if the browser says it's OK.
                CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
                if (browser) {
                    return browser->GetHost()->TryCloseBrowser();
                }
                return true;
            }

            CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
                return CefSize(800, 600);
            }

            cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
                return initial_show_state_;
            }

            cef_runtime_style_t GetWindowRuntimeStyle() override {
                return runtime_style_;
            }

        private:
            CefRefPtr<CefBrowserView> browser_view_;
            const cef_runtime_style_t runtime_style_;
            const cef_show_state_t initial_show_state_;

            IMPLEMENT_REFCOUNTING(CefWinDelegate);
            DISALLOW_COPY_AND_ASSIGN(CefWinDelegate);
        };

        class BrowserViewDelegate : public CefBrowserViewDelegate {
        public:
            explicit BrowserViewDelegate(cef_runtime_style_t runtime_style)
                : runtime_style_(runtime_style) {
            }

            bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                CefRefPtr<CefBrowserView> popup_browser_view,
                bool is_devtools) override {
                // Create a new top-level Window for the popup. It will show itself after
                // creation.
                CefWindow::CreateTopLevelWindow(new CefWinDelegate(
                    popup_browser_view, runtime_style_, CEF_SHOW_STATE_NORMAL));

                // We created the Window.
                return true;
            }

            cef_runtime_style_t GetBrowserRuntimeStyle() override {
                return runtime_style_;
            }

        private:
            const cef_runtime_style_t runtime_style_;

            IMPLEMENT_REFCOUNTING(BrowserViewDelegate);
            DISALLOW_COPY_AND_ASSIGN(BrowserViewDelegate);
        };

        CefNativeAppl::CefNativeAppl() {
            // Phase 6.3 Step 2: Create render delegate for JS bindings
            render_delegate_ = new cef_ui::renderer::CefRenderDelegate();
        }

        CefNativeAppl::~CefNativeAppl() {
            // Stop gRPC server before CEF shutdown
            if (grpc_server_) {
                grpc_server_->Stop();
            }
        }

        void CefNativeAppl::OnContextInitialized() {
            CEF_REQUIRE_UI_THREAD();

            CefRefPtr<CefCommandLine> command_line =
                CefCommandLine::GetGlobalCommandLine();

            // Phase 6.2: Start gRPC server
            std::string ipc_port_str = command_line->GetSwitchValue("ipcPort");
            std::string session_token = command_line->GetSwitchValue("sessionToken");
            
            if (!ipc_port_str.empty() && !session_token.empty()) {
                try {
                    uint16_t ipc_port = static_cast<uint16_t>(std::stoi(ipc_port_str));
                    grpc_server_ = std::make_unique<cef_ui::grpc_server::GrpcServer>(session_token);
                    
                    if (!grpc_server_->Start(ipc_port)) {
                        std::cerr << "[CefNativeAppl] Failed to start gRPC server" << std::endl;
                        grpc_server_.reset();
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[CefNativeAppl] Error starting gRPC server: " << e.what() << std::endl;
                }
            } else {
                std::cerr << "[CefNativeAppl] Warning: --ipcPort or --sessionToken not provided, gRPC server not started" << std::endl;
            }

            // Phase 6.2 Step 5: Schedule periodic command processing
            // Process commands every 100ms (fire-and-forget, no execution)
            if (grpc_server_ && grpc_server_->IsRunning()) {
                CefPostTask(TID_UI, 
                    base::BindOnce(&CefNativeAppl::ProcessPendingCommands, this));
            }

            static ui::CefUiBridgeImpl g_uiBridge;

            // Check if Alloy style will be used.
            cef_runtime_style_t runtime_style = CEF_RUNTIME_STYLE_DEFAULT;
            bool use_alloy_style = command_line->HasSwitch("use-alloy-style");
            if (use_alloy_style) {
                runtime_style = CEF_RUNTIME_STYLE_ALLOY;
            }

            // CefHandler implements browser-level callbacks.
            CefRefPtr<CefHandler> handler(new CefHandler(use_alloy_style, &g_uiBridge, this));

            // Specify CEF browser settings here.
            CefBrowserSettings browser_settings;

            std::string url;

            // Check if a "--url=" value was provided via the command-line. If so, use
            // that instead of the default URL.
            url = command_line->GetSwitchValue("url");
            if (url.empty()) {
                url = "http://10.255.1.143:8080/";
            }

            // Views is enabled by default (add `--use-native` to disable).
            const bool use_views = !command_line->HasSwitch("use-native");

            // If using Views create the browser using the Views framework, otherwise
            // create the browser using the native platform framework.
            if (use_views) {
                // Create the BrowserView.
                CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
                    handler, url, browser_settings, nullptr, nullptr,
                    new BrowserViewDelegate(runtime_style));

                // Optionally configure the initial show state.
                cef_show_state_t initial_show_state = CEF_SHOW_STATE_NORMAL;
                const std::string& show_state_value =
                    command_line->GetSwitchValue("initial-show-state");
                if (show_state_value == "minimized") {
                    initial_show_state = CEF_SHOW_STATE_MINIMIZED;
                }
                else if (show_state_value == "maximized") {
                    initial_show_state = CEF_SHOW_STATE_MAXIMIZED;
                }
#if defined(OS_MAC)
                // Hidden show state is only supported on MacOS.
                else if (show_state_value == "hidden") {
                    initial_show_state = CEF_SHOW_STATE_HIDDEN;
                }
#endif

                // Create the Window. It will show itself after creation.
                CefWindow::CreateTopLevelWindow(new CefWinDelegate(
                    browser_view, runtime_style, initial_show_state));
            }
            else {
                // Information used when creating the native window.
                CefWindowInfo window_info;

#if defined(OS_WIN)
                // On Windows we need to specify certain flags that will be passed to
                // CreateWindowEx().
                window_info.SetAsPopup(nullptr, "cefsimple");
#endif

                // Alloy style will create a basic native window. Chrome style will create a
                // fully styled Chrome UI window.
                window_info.runtime_style = runtime_style;

                // Create the first browser window.
                CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                    nullptr, nullptr);
                // Note: Browser reference will be stored in OnAfterCreated callback
            }
        }

        void CefNativeAppl::ProcessPendingCommands() {
            CEF_REQUIRE_UI_THREAD();

            // Phase 6.3 Step 1: Dequeue and EXECUTE commands using native CEF APIs
            if (!grpc_server_ || !grpc_server_->IsRunning()) {
                return;  // Server stopped, no more processing
            }

            cef_ui::grpc_server::CommandQueue* queue = grpc_server_->GetCommandQueue();
            if (!queue) {
                return;
            }

            // Process all pending commands
            while (auto cmd_opt = queue->Dequeue()) {
                const auto& cmd = cmd_opt.value();
                
                switch (cmd.GetType()) {
                    case cef_ui::grpc_server::CommandType::OPEN_PAGE: {
                        if (const auto* open_page = cmd.AsOpenPage()) {
                            std::cout << "[CefNativeAppl] ========== Executing OPEN_PAGE Command ==========" << std::endl;
                            std::cout << "[CefNativeAppl] command_id: " << open_page->command_id << std::endl;
                            std::cout << "[CefNativeAppl] url: " << open_page->url << std::endl;
                            
                            // Execute: Navigate browser to URL using native CEF API
                            if (browser_ && browser_->GetMainFrame()) {
                                browser_->GetMainFrame()->LoadURL(open_page->url);
                                std::cout << "[CefNativeAppl] Browser navigation INITIATED: " << open_page->url << std::endl;
                                std::cout << "[CefNativeAppl] (Load completion will be logged in OnLoadEnd)" << std::endl;
                            } else {
                                std::cerr << "[CefNativeAppl] FAILED: Browser not available" << std::endl;
                            }
                            std::cout << "[CefNativeAppl] ====================================================" << std::endl;
                        }
                        break;
                    }
                    case cef_ui::grpc_server::CommandType::SHUTDOWN: {
                        std::cout << "[CefNativeAppl] ========== Executing SHUTDOWN Command ==========" << std::endl;
                        
                        // Execute: Close browser cleanly
                        if (browser_) {
                            browser_->GetHost()->CloseBrowser(false);  // false = allow close handlers to run
                            std::cout << "[CefNativeAppl] Browser close INITIATED" << std::endl;
                            std::cout << "[CefNativeAppl] (Shutdown will complete in OnBeforeClose)" << std::endl;
                            // Note: CefQuitMessageLoop will be called in OnBeforeClose handler
                        } else {
                            std::cerr << "[CefNativeAppl] FAILED: Browser not available, quitting directly" << std::endl;
                            // If no browser, quit directly
                            CefQuitMessageLoop();
                        }
                        std::cout << "[CefNativeAppl] ====================================================" << std::endl;
                        break;
                    }
                }
            }

            // Schedule next processing cycle (100ms delay)
            CefPostDelayedTask(TID_UI, 
                base::BindOnce(&CefNativeAppl::ProcessPendingCommands, this), 
                100);
        }

        void CefNativeAppl::SetBrowser(CefRefPtr<CefBrowser> browser) {
            CEF_REQUIRE_UI_THREAD();
            
            // Phase 6.3: Store browser reference for command execution
            if (!browser_) {
                browser_ = browser;
                std::cout << "[CefNativeAppl] Browser reference stored for command execution" << std::endl;
            }
        }

        CefRefPtr<CefRenderProcessHandler> CefNativeAppl::GetRenderProcessHandler() {
            return render_delegate_;
        }

        CefRefPtr<CefClient> CefNativeAppl::GetDefaultClient() {
            // Called when a new browser window is created via Chrome style UI.
            return CefHandler::GetInstance();
        }
    }
}
