#include "../../inc/grpc/GrpcClient.h"
#include <iostream>
#include <windows.h>  // For GetCurrentProcessId

namespace cef_ui {
namespace grpc_client {

GrpcClient::GrpcClient(const std::string& server_address, const std::string& session_token)
    : server_address_(server_address),
      session_token_(session_token) {
}

GrpcClient::~GrpcClient() {
  Disconnect();
}

bool GrpcClient::ConnectAndHandshake() {
  if (is_connected_.load()) {
    std::cout << "[GrpcClient] Already connected" << std::endl;
    return true;
  }

  std::cout << "[GrpcClient] Connecting to Java server at " << server_address_ << std::endl;

  // Create gRPC channel to Java's server
  channel_ = grpc::CreateChannel(server_address_, grpc::InsecureChannelCredentials());
  
  if (!channel_) {
    std::cerr << "[GrpcClient] Failed to create gRPC channel" << std::endl;
    return false;
  }

  // Wait for channel to be ready (with timeout)
  auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
  if (!channel_->WaitForConnected(deadline)) {
    std::cerr << "[GrpcClient] Failed to connect to server within timeout" << std::endl;
    channel_.reset();
    return false;
  }

  std::cout << "[GrpcClient] Channel connected, creating stub" << std::endl;

  // Create stub for making RPC calls
  stub_ = cefcontrol::CefControlService::NewStub(channel_);
  
  if (!stub_) {
    std::cerr << "[GrpcClient] Failed to create service stub" << std::endl;
    channel_.reset();
    return false;
  }

  // Perform handshake
  if (!PerformHandshake()) {
    std::cerr << "[GrpcClient] Handshake failed" << std::endl;
    stub_.reset();
    channel_.reset();
    return false;
  }

  is_connected_.store(true);
  std::cout << "[GrpcClient] Connected and handshake successful" << std::endl;
  return true;
}

bool GrpcClient::PerformHandshake() {
  std::cout << "[GrpcClient] Sending handshake request" << std::endl;

  // Build handshake request
  cefcontrol::HandshakeRequest request;
  request.set_session_token(session_token_);
  request.set_client_version("1.0.0");
  
  // Add metadata
  (*request.mutable_metadata())["protocolVersion"] = "1.0";
  (*request.mutable_metadata())["parentPid"] = std::to_string(::GetCurrentProcessId());

  // Send handshake RPC
  cefcontrol::HandshakeResponse response;
  grpc::ClientContext context;
  
  // Set timeout for handshake
  auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
  context.set_deadline(deadline);

  grpc::Status status = stub_->Handshake(&context, request, &response);

  if (!status.ok()) {
    std::cerr << "[GrpcClient] Handshake RPC failed: " << status.error_message() << std::endl;
    return false;
  }

  if (!response.success()) {
    std::cerr << "[GrpcClient] Handshake rejected: " << response.message() << std::endl;
    return false;
  }

  std::cout << "[GrpcClient] Handshake accepted by server" << std::endl;
  std::cout << "[GrpcClient] Server version: " << response.server_version() << std::endl;
  std::cout << "[GrpcClient] Message: " << response.message() << std::endl;

  return true;
}

bool GrpcClient::IsConnected() const {
  return is_connected_.load();
}

void GrpcClient::Disconnect() {
  if (!is_connected_.load()) {
    return;
  }

  std::cout << "[GrpcClient] Disconnecting..." << std::endl;

  stub_.reset();
  channel_.reset();
  is_connected_.store(false);

  std::cout << "[GrpcClient] Disconnected" << std::endl;
}

cefcontrol::CefControlService::Stub* GrpcClient::GetStub() {
  if (!is_connected_.load()) {
    return nullptr;
  }
  return stub_.get();
}

}  // namespace grpc_client
}  // namespace cef_ui
