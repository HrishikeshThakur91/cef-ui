#pragma once

#include <memory>

namespace cef_ui {
    namespace ui {

        class NativeWindow;
        class CefBrowserManager;
        class ShutdownCoordinator;
    }

    namespace core {
        class AppConfig;

        // Forward declarations for control channel
        namespace core {
            class ControlCommandDispatcher;
        }
        namespace ipc {
            class FileEncryptedCommandReceiver;
        }

        /// Application-level root (Phase 5+)
        /// Owns UI objects, NOT CEF kernel lifecycle.
        class UIApplication {
        public:
            UIApplication(const core::AppConfig& config);
            ~UIApplication();

            // Non-copyable / non-movable
            UIApplication(const UIApplication&) = delete;
            UIApplication& operator=(const UIApplication&) = delete;
            UIApplication(UIApplication&&) = delete;
            UIApplication& operator=(UIApplication&&) = delete;

            /// Called AFTER CefInitialize()
            /// Must not call any CEF global init/shutdown APIs.
            void Start();

        private:
            const core::AppConfig& config_;
            std::unique_ptr<NativeWindow> window_;
            std::unique_ptr<CefBrowserManager> browser_;
            std::unique_ptr<ShutdownCoordinator> shutdown_;

            // TODO: Replace FileEncryptedCommandReceiver with gRPC receiver in Phase 6
            std::unique_ptr<core::ControlCommandDispatcher> command_dispatcher_;
            std::unique_ptr<ipc::FileEncryptedCommandReceiver> file_receiver_;
        };

    } // namespace ui
} // namespace cef_ui
