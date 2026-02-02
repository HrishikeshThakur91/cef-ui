#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <grpcpp/grpcpp.h>
#include "../../src/grpc/cef_service.grpc.pb.h"

namespace cef_ui {
namespace grpc_client {

/// gRPC client for connecting to Java's CefControlService server.
/// CEF acts as the client, Java acts as the server.
/// Thread-safe for connection operations.
class GrpcClient {
 public:
  /// Constructor.
  /// @param server_address Address of Java's gRPC server (e.g., "localhost:50051")
  /// @param session_token Session token for handshake validation
  explicit GrpcClient(const std::string& server_address, const std::string& session_token);
  
  ~GrpcClient();

  // Non-copyable, non-movable
  GrpcClient(const GrpcClient&) = delete;
  GrpcClient& operator=(const GrpcClient&) = delete;
  GrpcClient(GrpcClient&&) = delete;
  GrpcClient& operator=(GrpcClient&&) = delete;

  /// Connect to Java's gRPC server and perform handshake.
  /// Must be called before any other operations.
  /// @return true if connection and handshake successful
  bool ConnectAndHandshake();

  /// Check if client is connected and handshake completed.
  /// @return true if connected
  bool IsConnected() const;

  /// Disconnect from server and cleanup resources.
  /// Idempotent (safe to call multiple times).
  void Disconnect();

  /// Get the gRPC stub for making RPC calls.
  /// @return Pointer to stub, or nullptr if not connected
  cefcontrol::CefControlService::Stub* GetStub();

 private:
  std::string server_address_;
  std::string session_token_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<cefcontrol::CefControlService::Stub> stub_;
  std::atomic<bool> is_connected_{false};
  
  /// Perform handshake with Java server.
  /// @return true if handshake successful
  bool PerformHandshake();
};

}  // namespace grpc_client
}  // namespace cef_ui
