#include "../../inc/ipc/WssConnectionManager.h"
#include "../../inc/ipc/IpcProtocolException.h"
#include <algorithm>
#include <thread>
#include <chrono>
#include <stdexcept>

namespace cef_ui {
    namespace ipc {

        WssConnectionManager::WssConnectionManager(
            std::shared_ptr<IMessageChannel> channel)
            : channel_(channel),
            retry_count_(0),
            current_backoff_ms_(0),
            connected_(false) {
        }

        void WssConnectionManager::Connect() {
            if (connected_) {
                return;
            }

            retry_count_ = 0;
            EmitConnecting();

            while (retry_count_ < 5) {
                try {
                    // channel_ is IMessageChannel which doesn't directly connect
                    // For unit tests with mocks, they'll simulate connection via IsConnected()

                    // Force the channel to establish connectivity implicitly
                    if (!channel_->IsConnected()) {
                        // This must trigger underlying transport connect or fail
                        // NOTE: CRITICAL
                        // IMessageChannel does not expose an explicit Connect().
                        // Sending an empty message is used here to trigger
                        // underlying transport connection or failure.
                        channel_->Send("");   // no-op / handshake trigger
                    }
                    connected_ = true;
                    current_backoff_ms_ = 0;
                    EmitConnected();
                    return;
                }
                catch (const std::exception&) {
                    retry_count_++;
                    if (retry_count_ < 5) {
                        int backoff = CalculateNextBackoff();
                        Sleep(backoff);
                    }
                }
            }

            EmitError("Failed after 5 retries: Connection refused");
            throw IpcProtocolException("Failed to connect after 5 retries");
        }

        void WssConnectionManager::Disconnect() {
            if (connected_) {
                channel_->Close();
                connected_ = false;
                EmitDisconnected();
            }
        }

        bool WssConnectionManager::IsReady() const {
            return connected_ && channel_->IsConnected();
        }

        void WssConnectionManager::SendMessage(const std::string& message) {
            if (!IsReady()) {
                throw IpcProtocolException("Not connected");
            }
            channel_->Send(message);
        }

        std::string WssConnectionManager::ReceiveMessage() {
            if (!IsReady()) {
                throw IpcProtocolException("Not connected");
            }
            return channel_->Receive();
        }

        void WssConnectionManager::AddListener(std::shared_ptr<IConnectionListener> listener) {
            if (listener) {
                listeners_.push_back(listener);
            }
        }

        void WssConnectionManager::RemoveListener(std::shared_ptr<IConnectionListener> listener) {
            listeners_.erase(
                std::remove(listeners_.begin(), listeners_.end(), listener),
                listeners_.end());
        }

        WssConnectionManager::RetryStats WssConnectionManager::GetRetryStats() const {
            return { retry_count_, 5, current_backoff_ms_, 8000 };
        }

        int WssConnectionManager::GetConnectionTimeoutMs() const {
            return 30000;
        }

        int WssConnectionManager::CalculateNextBackoff() {
            static const int backoffs[] = { 1000, 2000, 4000, 8000, 8000 };
            if (retry_count_ < 5) {
                current_backoff_ms_ = backoffs[retry_count_];
                return current_backoff_ms_;
            }
            return 8000;
        }

        void WssConnectionManager::Sleep(int ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }

        void WssConnectionManager::EmitConnecting() {
            for (auto& listener : listeners_) {
                listener->OnConnecting();
            }
        }

        void WssConnectionManager::EmitConnected() {
            for (auto& listener : listeners_) {
                listener->OnConnected();
            }
        }

        void WssConnectionManager::EmitDisconnected() {
            for (auto& listener : listeners_) {
                listener->OnDisconnected();
            }
        }

        void WssConnectionManager::EmitError(const std::string& msg) {
            for (auto& listener : listeners_) {
                listener->OnError(msg);
            }
        }

    }  // namespace ipc
}  // namespace cef_ui
