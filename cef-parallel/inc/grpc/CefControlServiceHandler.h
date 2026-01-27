#pragma once

#include <grpcpp/grpcpp.h>
#include "cef_service.grpc.pb.h"

namespace cef_ui {
namespace ipc {

class CefControlServiceHandler : public cefcontrol::CefControlService::Service {
public:
    explicit CefControlServiceHandler(const std::string& session_token);
    ~CefControlServiceHandler() override = default;

    grpc::Status Handshake(grpc::ServerContext* context,
                           const cefcontrol::HandshakeRequest* request,
                           cefcontrol::HandshakeResponse* response) override;

    grpc::Status OpenPage(grpc::ServerContext* context,
                          const cefcontrol::OpenPageRequest* request,
                          cefcontrol::OpenPageResponse* response) override;

    grpc::Status PageStatus(grpc::ServerContext* context,
                            const cefcontrol::PageStatusRequest* request,
                            cefcontrol::PageStatusResponse* response) override;

    grpc::Status Shutdown(grpc::ServerContext* context,
                          const cefcontrol::ShutdownRequest* request,
                          cefcontrol::ShutdownResponse* response) override;

private:
    std::string expected_session_token_;
};

}  // namespace ipc
}  // namespace cef_ui
