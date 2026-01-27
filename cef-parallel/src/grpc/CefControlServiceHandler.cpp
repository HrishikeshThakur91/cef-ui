#include "../../cef-parallel/inc/grpc/CefControlServiceHandler.h"
#include <iostream>

namespace cef_ui {
namespace ipc {

CefControlServiceHandler::CefControlServiceHandler(const std::string& session_token)
    : expected_session_token_(session_token) {}

grpc::Status CefControlServiceHandler::Handshake(grpc::ServerContext* context,
                                                 const cefcontrol::HandshakeRequest* request,
                                                 cefcontrol::HandshakeResponse* response) {
    if (request->session_token() == expected_session_token_) {
        response->set_success(true);
        response->set_message("Handshake Accepted");
        response->set_server_version("1.0.0"); // TODO: Versioning
        return grpc::Status::OK;
    } else {
        response->set_success(false);
        response->set_message("Invalid Session Token");
        return grpc::Status::OK; // Contextually OK, but handshake failed
    }
}

grpc::Status CefControlServiceHandler::OpenPage(grpc::ServerContext* context,
                                                const cefcontrol::OpenPageRequest* request,
                                                cefcontrol::OpenPageResponse* response) {
    // Phase 6.2: Queue command, do not execute UI logic
    response->set_command_id(request->command_id());
    response->set_accepted(true); // Pretend we accepted it
    response->set_message("Command Queued (Phase 6.2 internal stub)");
    return grpc::Status::OK;
}

grpc::Status CefControlServiceHandler::PageStatus(grpc::ServerContext* context,
                                                  const cefcontrol::PageStatusRequest* request,
                                                  cefcontrol::PageStatusResponse* response) {
    response->set_command_id(request->command_id());
    response->set_status("UNKNOWN");
    response->set_message("Status Query Not Implemented in Phase 6.2");
    return grpc::Status::OK;
}

grpc::Status CefControlServiceHandler::Shutdown(grpc::ServerContext* context,
                                                const cefcontrol::ShutdownRequest* request,
                                                cefcontrol::ShutdownResponse* response) {
    response->set_acknowledged(true);
    response->set_message("Shutdown ignored in Phase 6.2");
    return grpc::Status::OK;
}

}  // namespace ipc
}  // namespace cef_ui
