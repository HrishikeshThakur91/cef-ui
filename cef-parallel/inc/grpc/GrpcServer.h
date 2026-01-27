#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <grpcpp/grpcpp.h>

namespace cef_ui {
namespace grpc_server {

// Forward declarations
class CefControlServiceImpl;
class CommandQueue;

/// Manages gRPC server lifecycle for the CEF process.
/// Owned by CefNativeAppl.
/// Thread-safe for Start/Stop operations.
class GrpcServer {
 public:
  /// Constructor.
  /// @param session_token Expected session token for handshake validation
  explicit GrpcServer(const std::string& session_token);
  
  ~GrpcServer();

  // Non-copyable, non-movable (owns unique resources)
  GrpcServer(const GrpcServer&) = delete;
  GrpcServer& operator=(const GrpcServer&) = delete;
  GrpcServer(GrpcServer&&) = delete;
  GrpcServer& operator=(GrpcServer&&) = delete;

  /// Start the gRPC server on the specified port.
  /// Must be called from CEF UI thread (in OnContextInitialized).
  /// @param port Port number to bind to (e.g., from --ipcPort)
  /// @return true if server started successfully, false otherwise
  bool Start(uint16_t port);

  /// Stop the gRPC server and wait for shutdown.
  /// Blocks until all in-flight RPCs complete.
  /// Safe to call from any thread.
  /// Idempotent (safe to call multiple times).
  void Stop();

  /// Check if server is currently running.
  /// @return true if server is running
  bool IsRunning() const;

  /// Check if server is shutting down.
  /// Used by RPC handlers to reject new commands.
  /// @return true if shutdown has been initiated
  bool IsShuttingDown() const;

  /// Get the command queue for posting commands to UI thread
  /// @return Pointer to command queue (non-null if server is running)
  CommandQueue* GetCommandQueue();

 private:
  std::string session_token_;
  std::unique_ptr<CefControlServiceImpl> service_impl_;
  std::unique_ptr<grpc::Server> server_;
  std::unique_ptr<CommandQueue> command_queue_;
  std::atomic<bool> is_running_{false};
  std::atomic<bool> is_shutting_down_{false};
};

}  // namespace grpc_server
}  // namespace cef_ui
