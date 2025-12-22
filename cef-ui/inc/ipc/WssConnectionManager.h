#pragma once

#include <string>
#include <memory>
#include <vector>

#include "IMessageChannel.h"

namespace cef_ui {
    namespace ipc {

        /// Observer pattern for connection lifecycle events.
        class IConnectionListener {
        public:
            virtual ~IConnectionListener() = default;

            virtual void OnConnecting() = 0;
            virtual void OnConnected() = 0;
            virtual void OnDisconnected() = 0;
            virtual void OnError(const std::string& error_msg) = 0;
        };

        /// Manage WSS connection lifecycle with exponential backoff retry strategy.
        /// 
        /// Responsibilities:
        /// - Establish WSS connection with retries (max 5, exponential backoff)
        /// - Emit lifecycle events (OnConnecting, OnConnected, OnDisconnected, OnError)
        /// - Track retry statistics and apply 30s timeout
        /// - Send/receive messages through IMessageChannel
        /// 
        /// Guarantees:
        /// - Exponential backoff: 1s ? 2s ? 4s ? 8s ? 8s (max)
        /// - Max 5 retries before throwing IpcProtocolException
        /// - 30 second connection timeout
        /// - Error messages include retry count
        /// - Deterministic, no threads
        class WssConnectionManager {
        public:
            /// Constructor: inject message channel (dependency inversion).
            /// @param channel: IMessageChannel implementation (WebSocket, mock, etc)
            explicit WssConnectionManager(std::shared_ptr<IMessageChannel> channel);

            ~WssConnectionManager() = default;

            // Non-copyable
            WssConnectionManager(const WssConnectionManager&) = delete;
            WssConnectionManager& operator=(const WssConnectionManager&) = delete;

            /// Connect with exponential backoff retry strategy.
            /// @throws IpcProtocolException after 5 failed retries
            void Connect();

            /// Graceful disconnect.
            void Disconnect();

            /// Check connection readiness.
            /// @return true if connected and ready for I/O
            bool IsReady() const;

            /// Send message through channel.
            /// @param message: JSON message string
            /// @throws IpcProtocolException if not connected or send fails
            void SendMessage(const std::string& message);

            /// Receive message from channel.
            /// @return JSON message string
            /// @throws IpcProtocolException if receive fails
            std::string ReceiveMessage();

            /// Register listener for connection lifecycle events.
            /// @param listener: observer to receive OnConnecting, OnConnected, etc.
            void AddListener(std::shared_ptr<IConnectionListener> listener);

            /// Remove listener from event notifications.
            /// @param listener: observer to stop receiving events
            void RemoveListener(std::shared_ptr<IConnectionListener> listener);

            /// Retrieve retry statistics for diagnostics.
            /// @return RetryStats with retry_count, max_retries, backoff values
            struct RetryStats {
                int retry_count = 0;
                int max_retries = 5;
                int current_backoff_ms = 0;
                int max_backoff_ms = 8000;
            };
            RetryStats GetRetryStats() const;

            /// Get connection timeout in milliseconds.
            /// @return 30000 (30 seconds)
            int GetConnectionTimeoutMs() const;

        private:
            std::shared_ptr<IMessageChannel> channel_;
            std::vector<std::shared_ptr<IConnectionListener>> listeners_;
            int retry_count_;
            int current_backoff_ms_;
            bool connected_;

            /// Helper: calculate next backoff value (exponential: 1s, 2s, 4s, 8s, 8s max).
            /// @return backoff in milliseconds
            int CalculateNextBackoff();

            /// Helper: sleep without blocking (virtual for mocking in tests).
            /// @param ms: milliseconds to sleep
            virtual void Sleep(int ms);

            /// Helper: emit lifecycle events to all registered listeners.
            void EmitConnecting();
            void EmitConnected();
            void EmitDisconnected();
            void EmitError(const std::string& msg);
        };

    }  // namespace ipc
}  // namespace cef_ui