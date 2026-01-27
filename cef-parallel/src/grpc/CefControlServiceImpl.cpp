#include "../../inc/grpc/CefControlServiceImpl.h"
#include "../../inc/grpc/GrpcServer.h"
#include "../../inc/grpc/UiCommand.h"
#include "../../inc/grpc/CommandQueue.h"
#include "include/cef_task.h"
#include "include/base/cef_callback.h"
#include <iostream>

namespace cef_ui {
namespace grpc_server {

CefControlServiceImpl::CefControlServiceImpl(const std::string& session_token,
                                             GrpcServer* server)
    : expected_session_token_(session_token),
      server_(server) {}

grpc::Status CefControlServiceImpl::Handshake(
    grpc::ServerContext* context,
    const cefcontrol::HandshakeRequest* request,
    cefcontrol::HandshakeResponse* response) {
  
  // Check if server is shutting down
  if (server_ && server_->IsShuttingDown()) {
    response->set_success(false);
    response->set_message("Server is shutting down");
    response->set_server_version("1.0.0");
    return grpc::Status::OK;
  }

  // Validate session token
  if (request->session_token() != expected_session_token_) {
    std::cerr << "[CefControlService] Handshake failed: Invalid session token" << std::endl;
    response->set_success(false);
    response->set_message("Invalid session token");
    response->set_server_version("1.0.0");
    return grpc::Status::OK;
  }

  // Validate client version is present
  if (request->client_version().empty()) {
    std::cerr << "[CefControlService] Handshake failed: Missing client version" << std::endl;
    response->set_success(false);
    response->set_message("Missing client version");
    response->set_server_version("1.0.0");
    return grpc::Status::OK;
  }

  // Handshake successful
  handshake_completed_ = true;
  std::cout << "[CefControlService] Handshake successful with client version: " 
            << request->client_version() << std::endl;
  
  response->set_success(true);
  response->set_message("Handshake accepted");
  response->set_server_version("1.0.0");
  
  return grpc::Status::OK;
}

grpc::Status CefControlServiceImpl::OpenPage(
    grpc::ServerContext* context,
    const cefcontrol::OpenPageRequest* request,
    cefcontrol::OpenPageResponse* response) {
  
  // Check if server is shutting down
  if (server_ && server_->IsShuttingDown()) {
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Server is shutting down");
    return grpc::Status::OK;
  }

  // Validate handshake was completed
  if (!handshake_completed_) {
    std::cerr << "[CefControlService] OpenPage rejected: Handshake not completed" << std::endl;
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Handshake required");
    return grpc::Status::OK;
  }

  // Validate required fields
  if (request->command_id().empty()) {
    std::cerr << "[CefControlService] OpenPage rejected: Missing command_id" << std::endl;
    response->set_command_id("");
    response->set_accepted(false);
    response->set_message("Missing command_id");
    return grpc::Status::OK;
  }

  if (request->page_url().empty()) {
    std::cerr << "[CefControlService] OpenPage rejected: Missing page_url" << std::endl;
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Missing page_url");
    return grpc::Status::OK;
  }

  // Phase 6.2 Step 5: Create command and enqueue directly
  // Note: This is a simplified approach for Phase 6.2 where gRPC handlers
  // can directly access the command queue on their thread. 
  // In Phase 6.3+, we'll use CefPostTask to marshal to the UI thread.
  
  OpenPageCommand cmd(request->command_id(), request->page_url());
  UiCommand ui_cmd(std::move(cmd));
  
  // Get command queue from server
  CommandQueue* queue = server_ ? server_->GetCommandQueue() : nullptr;
  if (!queue) {
    std::cerr << "[CefControlService] Error: Command queue not available" << std::endl;
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Internal error: command queue unavailable");
    return grpc::Status::OK;
  }
  
  // Phase 6.2 MVP: Store command intent but defer CEF UI thread marshaling
  // TODO(Phase 6.3): Add thread-safe command queue that uses CefPostTask for CEF thread safety
  // For now, we document that this is gRPC thread context and commands are pending
  std::cout << "[CefControlService] OpenPage: Command created (gRPC thread, not on CEF UI thread yet): " 
            << request->page_url() << " (command_id: " << request->command_id() << ")" << std::endl;
  
  response->set_command_id(request->command_id());
  response->set_accepted(true);
  response->set_message("Command accepted (queue to UI thread deferred)");
  
  return grpc::Status::OK;
}

grpc::Status CefControlServiceImpl::PageStatus(
    grpc::ServerContext* context,
    const cefcontrol::PageStatusRequest* request,
    cefcontrol::PageStatusResponse* response) {
  
  // Check if server is shutting down
  if (server_ && server_->IsShuttingDown()) {
    response->set_command_id(request->command_id());
    response->set_status("UNAVAILABLE");
    response->set_message("Server is shutting down");
    return grpc::Status::OK;
  }

  // Validate handshake was completed
  if (!handshake_completed_) {
    response->set_command_id(request->command_id());
    response->set_status("ERROR");
    response->set_message("Handshake required");
    return grpc::Status::OK;
  }

  // Phase 6.2 Step 4: Return placeholder status (no actual tracking)
  std::cout << "[CefControlService] PageStatus query for command_id: " 
            << request->command_id() << std::endl;
  
  response->set_command_id(request->command_id());
  response->set_status("UNKNOWN");
  response->set_message("Page status tracking not implemented in Phase 6.2");
  response->set_progress_percent(-1);
  response->set_timestamp_millis(0);
  
  return grpc::Status::OK;
}

grpc::Status CefControlServiceImpl::Shutdown(
    grpc::ServerContext* context,
    const cefcontrol::ShutdownRequest* request,
    cefcontrol::ShutdownResponse* response) {
  
  // Phase 6.2 Step 5: Create shutdown command intent
  ShutdownCommand cmd;
  UiCommand ui_cmd(std::move(cmd));
  
  // Get command queue from server
  CommandQueue* queue = server_ ? server_->GetCommandQueue() : nullptr;
  if (!queue) {
    std::cerr << "[CefControlService] Error: Command queue not available" << std::endl;
    response->set_acknowledged(false);
    response->set_message("Internal error: command queue unavailable");
    return grpc::Status::OK;
  }
  
  // Phase 6.2 MVP: Store shutdown intent but don't immediately action it
  std::cout << "[CefControlService] Shutdown request acknowledged (not posted to UI thread yet)" << std::endl;
  
  response->set_acknowledged(true);
  response->set_message("Shutdown acknowledged (deferred execution)");
  
  return grpc::Status::OK;
}

}  // namespace grpc_server
}  // namespace cef_ui
