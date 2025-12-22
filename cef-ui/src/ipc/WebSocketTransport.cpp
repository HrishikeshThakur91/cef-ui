#include "../../inc/ipc/WebSocketTransport.h"
#include "../../inc/ipc/IpcProtocolException.h"
#include <stdexcept>

namespace cef_ui {
    namespace ipc {

        WebSocketTransport::WebSocketTransport(
            std::shared_ptr<IWebSocketConnection> connection,
            std::shared_ptr<ITlsContextProvider> tls_provider,
            const std::string& host,
            uint16_t port)
            : connection_(connection),
            tls_provider_(tls_provider),
            host_(host),
            port_(port),
            connected_(false) {
        }

        void WebSocketTransport::Connect() {
            if (!tls_provider_->IsAvailable()) {
                throw IpcProtocolException("TLS provider not available");
            }

            if (connected_) {
                return;
            }

            auto tls_context = CreateSecureTlsContext();
            connection_->Connect(host_, port_, *tls_context);
            connected_ = true;
        }

        void WebSocketTransport::Send(const std::string& message) {
            if (!connected_) {
                throw IpcProtocolException("Not connected");
            }
            connection_->Send(message);
        }

        std::string WebSocketTransport::Receive() {
            if (!connected_) {
                throw IpcProtocolException("Not connected");
            }
            return connection_->Receive();
        }

        bool WebSocketTransport::IsConnected() const {
            return connected_ && connection_->IsConnected();
        }

        void WebSocketTransport::Close() {
            if (connected_) {
                connection_->Close();
                connected_ = false;
            }
        }

        std::unique_ptr<ITlsContext> WebSocketTransport::CreateSecureTlsContext() {
            if (!tls_provider_) {
                throw IpcProtocolException("TLS provider not available");
            }
            return tls_provider_->CreateTlsContext(host_);
        }

    }  // namespace ipc
}  // namespace cef_ui
