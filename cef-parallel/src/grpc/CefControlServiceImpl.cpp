#include "../../inc/grpc/CefControlServiceImpl.h"
#include "../../inc/grpc/GrpcServer.h"
#include "../../inc/grpc/UiCommand.h"
#include "../../inc/grpc/CommandQueue.h"
#include "include/cef_task.h"
#include "include/base/cef_callback.h"
#include "include/wrapper/cef_closure_task.h"
#include <iostream>

namespace cef_ui {
namespace grpc_server {

CefControlServiceImpl::CefControlServiceImpl(const std::string& session_token,
                                             GrpcServer* server,
                                             const std::string& java_callback_address)
    : expected_session_token_(session_token),
      server_(server),
      java_callback_address_(java_callback_address) {
  
  // Phase 6.3: Initialize gRPC channel to Java's callback service if address provided
  if (!java_callback_address_.empty()) {
    java_callback_channel_ = grpc::CreateChannel(
        java_callback_address_,
        grpc::InsecureChannelCredentials());
    
    std::cout << "[CefControlService] Initialized status callback channel to: " 
              << java_callback_address_ << std::endl;
    
    // Note: Stub creation will be added once proto files are regenerated
    // status_callback_stub_ = cefcontrol::CefStatusCallbackService::NewStub(java_callback_channel_);
  }
}

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
  
  // Phase 6.3 Step 3: Enhanced validation logging
  std::cout << "[CefControlService] ========== OpenPage Request Received ==========" << std::endl;
  std::cout << "[CefControlService] command_id: " << request->command_id() << std::endl;
  std::cout << "[CefControlService] page_url: " << request->page_url() << std::endl;
  
  // Check if server is shutting down
  if (server_ && server_->IsShuttingDown()) {
    std::cerr << "[CefControlService] REJECTED: Server is shutting down" << std::endl;
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Server is shutting down");
    return grpc::Status::OK;
  }

  // Validate handshake
  if (!handshake_completed_) {
    std::cerr << "[CefControlService] REJECTED: Handshake not completed" << std::endl;
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Handshake not completed");
    return grpc::Status::OK;
  }

  // Validate required fields
  if (request->command_id().empty()) {
    std::cerr << "[CefControlService] REJECTED: Missing command_id" << std::endl;
    response->set_command_id("");
    response->set_accepted(false);
    response->set_message("Missing command_id");
    return grpc::Status::OK;
  }

  if (request->page_url().empty()) {
    std::cerr << "[CefControlService] REJECTED: Missing page_url" << std::endl;
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Missing page_url");
    return grpc::Status::OK;
  }

  // Phase 6.2 Step 5: Create command and store for UI thread processing
  std::cout << "[CefControlService] Creating UiCommand (command_id: " << request->command_id() << ")" << std::endl;
  OpenPageCommand cmd(request->command_id(), request->page_url());
  UiCommand ui_cmd(std::move(cmd));
  
  // Get command queue from server
  CommandQueue* queue = server_ ? server_->GetCommandQueue() : nullptr;
  if (!queue) {
    std::cerr << "[CefControlService] ERROR: Command queue not available" << std::endl;
    response->set_command_id(request->command_id());
    response->set_accepted(false);
    response->set_message("Internal error: command queue unavailable");
    return grpc::Status::OK;
  }
  
  std::cout << "[CefControlService] Posting command to UI thread (command_id: " << request->command_id() << ")" << std::endl;
  
  // Post command to CEF UI thread using CefPostTask
  // Fire-and-forget: we don't wait for execution
  CefPostTask(TID_UI, base::BindOnce([](CommandQueue* q, UiCommand cmd) {
      q->Enqueue(std::move(cmd));
      }, queue, std::move(ui_cmd)));

  std::cout << "[CefControlService] OpenPage ACCEPTED and queued (command_id: " << request->command_id() << ")" << std::endl;
  std::cout << "[CefControlService] ====================================================" << std::endl;

  response->set_command_id(request->command_id());
  response->set_accepted(true);
  response->set_message("Command accepted and queued for execution");

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
  
  // Phase 6.3 Step 3: Enhanced validation logging
  std::cout << "[CefControlService] ========== Shutdown Request Received ==========" << std::endl;
  
    // Phase 6.3: Create shutdown command and post to UI thread for execution
  ShutdownCommand cmd;
  UiCommand ui_cmd(std::move(cmd));
  
  // Get command queue from server
  CommandQueue* queue = server_ ? server_->GetCommandQueue() : nullptr;
  if (!queue) {
    std::cerr << "[CefControlService] ERROR: Command queue not available" << std::endl;
    response->set_acknowledged(false);
    response->set_message("Internal error: command queue unavailable");
    return grpc::Status::OK;
  }
  
  std::cout << "[CefControlService] Posting shutdown command to UI thread" << std::endl;
  
  // Post command to CEF UI thread using CefPostTask
  CefPostTask(TID_UI, base::BindOnce([](CommandQueue* q, UiCommand cmd) {
      q->Enqueue(std::move(cmd));
      }, queue, std::move(ui_cmd)));

  std::cout << "[CefControlService] Shutdown ACKNOWLEDGED and queued" << std::endl;
  std::cout << "[CefControlService] ====================================================" << std::endl;

  response->set_acknowledged(true);
  response->set_message("Shutdown acknowledged and queued for execution");
  
  return grpc::Status::OK;
}

// Phase 6.3: Send status notification to Java's callback service
void CefControlServiceImpl::SendStatusNotification(
    const std::string& command_id,
    const std::string& status,
    const std::string& message,
    int progress_percent) {
  
  // Check if callback channel is initialized
  if (!java_callback_channel_) {
    std::cerr << "[CefControlService] WARNING: Status callback channel not initialized, "
              << "cannot send status notification" << std::endl;
    return;
  }

  std::cout << "[CefControlService] Sending status notification: command_id=" << command_id
            << ", status=" << status << ", message=" << message 
            << ", progress=" << progress_percent << "%" << std::endl;

  // TODO: Once proto files are regenerated with CefStatusCallbackService:
  //
  // 1. Create stub if not already created:
  //    if (!status_callback_stub_) {
  //      status_callback_stub_ = cefcontrol::CefStatusCallbackService::NewStub(java_callback_channel_);
  //    }
  //
  // 2. Build PageStatusNotification:
  //    cefcontrol::PageStatusNotification notification;
  //    notification.set_command_id(command_id);
  //    notification.set_status(status);
  //    notification.set_message(message);
  //    notification.set_progress_percent(progress_percent);
  //    notification.set_timestamp_millis(
  //        std::chrono::duration_cast<std::chrono::milliseconds>(
  //            std::chrono::system_clock::now().time_since_epoch()).count());
  //
  // 3. Call Java's callback service:
  //    cefcontrol::StatusAck ack;
  //    grpc::ClientContext context;
  //    
  //    grpc::Status rpc_status = status_callback_stub_->NotifyPageStatus(
  //        &context, notification, &ack);
  //    
  //    if (!rpc_status.ok()) {
  //      std::cerr << "[CefControlService] Failed to send status notification: "
  //                << rpc_status.error_message() << std::endl;
  //    } else if (!ack.received()) {
  //      std::cerr << "[CefControlService] Status notification rejected: "
  //                << ack.error_message() << std::endl;
  //    } else {
  //      std::cout << "[CefControlService] Status notification acknowledged by Java" << std::endl;
  //    }
  
  // For now, just log that we would send the notification
  std::cout << "[CefControlService] Status notification prepared (waiting for proto regeneration)" << std::endl;
}

}  // namespace grpc_server
}  // namespace cef_ui
