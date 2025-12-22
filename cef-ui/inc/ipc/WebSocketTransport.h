#pragma once

#include "IMessageChannel.h"
#include "ITlsContextProvider.h"
#include <memory>
#include <string>

// Add the correct namespace for IMessageChannel
namespace cef_ui {
    namespace ipc {

        class WebSocketTransport : public IMessageChannel {
        public:
            // Constructor: inject dependencies
            // @param connection: WebSocket I/O implementation (not owned)
            // @param tls_provider: TLS context factory (not owned)
            // @param host: remote host to connect to
            // @param port: remote port to connect to
            explicit WebSocketTransport(
                std::shared_ptr<IWebSocketConnection> connection,
                std::shared_ptr<ITlsContextProvider> tls_provider,
                const std::string& host,
                uint16_t port);

            ~WebSocketTransport() override = default;

            // IMessageChannel implementation
            void Send(const std::string& message) override;
            std::string Receive() override;
            bool IsConnected() const override;
            void Close() override;

            // Phase 4 specific: establish WSS connection
            // @throws IpcProtocolException on TLS or network error
            void Connect();

        private:
            std::shared_ptr<IWebSocketConnection> connection_;
            std::shared_ptr<ITlsContextProvider> tls_provider_;
            std::string host_;
            uint16_t port_;
            bool connected_;

            // Helper: create and configure TLS context
            std::unique_ptr<ITlsContext> CreateSecureTlsContext();
        };

    }  // namespace ipc
}  // namespace cef_ui