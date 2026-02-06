#include "../../inc/ui/UIApplication.h"
#include "../../inc/ui/NativeWindow.h"
#include "../../inc/ui/CefBrowserManager.h"
#include "../../inc/ui/ShutdownCoordinator.h"
#include "../../inc/core/AppConfig.h"
#include "../../inc/core/ControlCommandDispatcher.h"
#include "../../inc/ipc/FileEncryptedCommandReceiver.h"
#include <iostream>
#include <filesystem>

namespace cef_ui {
    namespace ui {

        UIApplication::UIApplication(const core::AppConfig& config)
            : config_(config) {}

        UIApplication::~UIApplication() {
            // Ensure file receiver polling thread is stopped
            if (file_receiver_) {
                file_receiver_->Stop();
            }
            // Note: ControlCommandDispatcher owns no resources and requires no explicit shutdown
        }

        void UIApplication::Start() {
            // Phase-5 rule:
            // - No CefInitialize
            // - No CefRunMessageLoop
            // - No CefShutdown

            // 1. Create native window (hidden initially)
            window_ = std::make_unique<NativeWindow>("CEF UI");

            // 2. Create browser and load UI
            const std::string ui_url = "https://www.google.com/"; // keep as-is for now
            browser_ = std::make_unique<CefBrowserManager>(*window_, ui_url);

            // 3. Setup shutdown coordination
            shutdown_ = std::make_unique<ShutdownCoordinator>();

            // TODO: Replace FileEncryptedCommandReceiver with gRPC receiver in Phase 6
            // 4. Setup file-based control channel (if configured)
            // Note: This is temporary - will be replaced with gRPC in Phase 6
            const std::string& control_file = config_.GetControlFile();
            const std::string& control_key = config_.GetControlKey();

            if (!control_file.empty() && !control_key.empty()) {
                std::cerr << "[UIApplication] Initializing file-based control channel..." << std::endl;
                std::cerr << "[UIApplication] Control file: " << control_file << std::endl;

                try {
                    // Create dispatcher
                    command_dispatcher_ = std::make_unique<cef_ui::core::ControlCommandDispatcher>(
                        browser_->GetBrowserInstance(),
                        *shutdown_
                    );

                    // Create file receiver
                    file_receiver_ = std::make_unique<cef_ui::ipc::FileEncryptedCommandReceiver>(
                        std::filesystem::path(control_file),
                        control_key,
                        *command_dispatcher_
                    );

                    // Start polling in background thread (non-blocking)
                    file_receiver_->Start();
                    std::cerr << "[UIApplication] File-based control channel started" << std::endl;

                } catch (const std::exception& e) {
                    std::cerr << "[UIApplication] Failed to initialize control channel: "
                              << e.what() << std::endl;
                    // Continue without control channel
                }
            } else {
                std::cerr << "[UIApplication] Control channel not configured, skipping" << std::endl;
            }

            // NOTE:
            // Showing window, IPC, crash handling -> Phase 6/7
        }

    } // namespace ui
} // namespace cef_ui
