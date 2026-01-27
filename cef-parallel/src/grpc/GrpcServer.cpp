#include "../../inc/grpc/GrpcServer.h"
#include "../../inc/grpc/CefControlServiceImpl.h"
#include "../../inc/grpc/CommandQueue.h"
#include <sstream>
#include <iostream>

namespace cef_ui {
namespace grpc_server {

GrpcServer::GrpcServer(const std::string& session_token)
    : session_token_(session_token),
      command_queue_(std::make_unique<CommandQueue>()) {
  // Phase 6.2 Step 5: Command queue created
}

GrpcServer::~GrpcServer() {
  Stop();
}

bool GrpcServer::Start(uint16_t port) {
  // Prevent double-start
  if (is_running_.load()) {
    // Log error to stderr (no CEF dependencies)
    std::cerr << "[GrpcServer] Error: Start() called but server is already running" << std::endl;
    return false;
  }

  // Create service implementation
  service_impl_ = std::make_unique<CefControlServiceImpl>(session_token_, this);

  // Build server address
  std::stringstream address_ss;
  address_ss << "0.0.0.0:" << port;
  std::string server_address = address_ss.str();

  // Build and start gRPC server
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  
  // Register service implementation
  builder.RegisterService(service_impl_.get());

  server_ = builder.BuildAndStart();

  if (!server_) {
    std::cerr << "[GrpcServer] Error: Failed to start gRPC server on " << server_address << std::endl;
    service_impl_.reset();
    return false;
  }

  std::cout << "[GrpcServer] Started on " << server_address << std::endl;
  is_running_.store(true);
  return true;
}

void GrpcServer::Stop() {
  // Idempotent: safe to call multiple times
  if (!is_running_.load()) {
    return;
  }

  std::cout << "[GrpcServer] Stopping..." << std::endl;

  // Set shutdown flag FIRST - prevents new commands from being posted
  is_shutting_down_.store(true);

  // Shutdown gRPC server (blocks until all RPCs complete)
  if (server_) {
    server_->Shutdown();
    // Wait for server to fully stop
    server_->Wait();
    server_.reset();
  }

  // Clean up service implementation
  service_impl_.reset();

  is_running_.store(false);
  std::cout << "[GrpcServer] Stopped" << std::endl;
}

bool GrpcServer::IsRunning() const {
  return is_running_.load();
}

bool GrpcServer::IsShuttingDown() const {
  return is_shutting_down_.load();
}

CommandQueue* GrpcServer::GetCommandQueue() {
  return command_queue_.get();
}

}  // namespace grpc_server
}  // namespace cef_ui
