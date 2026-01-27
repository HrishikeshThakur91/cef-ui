#pragma once

#include <string>
#include <grpcpp/grpcpp.h>
#include "cef_service.grpc.pb.h"

namespace cef_ui {
namespace grpc_server {

// Forward declaration
class GrpcServer;

/// Implementation of CefControlService gRPC service.
/// Handles incoming RPC calls from Java control plane.
/// 
/// Phase 6.2 Step 4: Validates requests and returns acceptance/rejection only.
/// Does NOT execute UI behavior or post CEF tasks.
///
/// Thread-safety: All methods execute on gRPC threads (non-CEF).
class CefControlServiceImpl : public cefcontrol::CefControlService::Service {
 public:
  /// Constructor.
  /// @param session_token Expected session token for handshake validation
  /// @param server Pointer to GrpcServer (for shutdown flag checking)
  CefControlServiceImpl(const std::string& session_token, GrpcServer* server);

  ~CefControlServiceImpl() override = default;

  // Non-copyable, non-movable
  CefControlServiceImpl(const CefControlServiceImpl&) = delete;
  CefControlServiceImpl& operator=(const CefControlServiceImpl&) = delete;

  /// Handshake RPC handler.
  /// Validates session token and returns success/failure.
  grpc::Status Handshake(grpc::ServerContext* context,
                         const cefcontrol::HandshakeRequest* request,
                         cefcontrol::HandshakeResponse* response) override;

  /// OpenPage RPC handler.
  /// Validates request and returns acceptance (does NOT execute UI action).
  grpc::Status OpenPage(grpc::ServerContext* context,
                        const cefcontrol::OpenPageRequest* request,
                        cefcontrol::OpenPageResponse* response) override;

  /// PageStatus RPC handler.
  /// Returns placeholder status (no actual page tracking yet).
  grpc::Status PageStatus(grpc::ServerContext* context,
                          const cefcontrol::PageStatusRequest* request,
                          cefcontrol::PageStatusResponse* response) override;

  /// Shutdown RPC handler.
  /// Acknowledges shutdown request (does NOT trigger actual shutdown).
  grpc::Status Shutdown(grpc::ServerContext* context,
                        const cefcontrol::ShutdownRequest* request,
                        cefcontrol::ShutdownResponse* response) override;

 private:
  std::string expected_session_token_;
  GrpcServer* server_;  // Non-owning pointer for shutdown flag checking
  bool handshake_completed_ = false;  // Track if handshake succeeded
};

}  // namespace grpc_server
}  // namespace cef_ui
