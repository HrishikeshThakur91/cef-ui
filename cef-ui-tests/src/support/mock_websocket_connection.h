#pragma once

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include "ipc/IWebSocketConnection.h"
#include "ipc/IMessageChannel.h"
#include "ipc/IpcProtocolException.h"

namespace cef_ui {
namespace ipc {

/// Mock WebSocket connection for unit testing.
/// Implements both IWebSocketConnection and IMessageChannel.
/// Allows injection of canned responses and failure simulation.
class MockWebSocketConnection : public IWebSocketConnection, public IMessageChannel {
 public:
  MockWebSocketConnection() : connected_(false), connect_should_fail_(false),
                              send_should_fail_(false), receive_should_fail_(false),
                              connect_call_count_(0), close_call_count_(0) {}

  // Test configuration
  void SetNextResponse(const std::string& response) {
    responses_.push(response);
  }

  void SetConnectFailure(const std::string& error) {
    connect_should_fail_ = true;
    connect_error_ = error;
  }

  void SetSendFailure(const std::string& error) {
    send_should_fail_ = true;
    send_error_ = error;
  }

  void SetReceiveFailure(const std::string& error) {
    receive_should_fail_ = true;
    receive_error_ = error;
  }

  void SetConnectSuccess() {
    connect_should_fail_ = false;
  }

  void SimulateDisconnect() {
    connected_ = false;
  }

  // Call tracking
  std::vector<std::string> GetSentMessages() const {
    return sent_messages_;
  }

  int GetConnectCallCount() const {
    return connect_call_count_;
  }

  int GetCloseCallCount() const {
    return close_call_count_;
  }

  std::vector<std::string> GetConnectHosts() const {
    return connect_hosts_;
  }

  std::vector<uint16_t> GetConnectPorts() const {
    return connect_ports_;
  }

  // IWebSocketConnection implementation
  void Connect(const std::string& host, uint16_t port,
               const ITlsContext& tls_context) override {
    connect_call_count_++;
    connect_hosts_.push_back(host);
    connect_ports_.push_back(port);
    
    if (connect_should_fail_) {
      throw IpcProtocolException(connect_error_);
    }
    connected_ = true;
  }

  void Send(const std::string& message) override {
    if (!connected_) {
      throw IpcProtocolException("Not connected");
    }
    if (send_should_fail_) {
      throw IpcProtocolException(send_error_);
    }
    sent_messages_.push_back(message);
  }

  std::string Receive() override {
    if (!connected_) {
      throw IpcProtocolException("Not connected");
    }
    if (receive_should_fail_) {
      throw IpcProtocolException(receive_error_);
    }
    if (responses_.empty()) {
      throw IpcProtocolException("No response available");
    }
    std::string response = responses_.front();
    responses_.pop();
    return response;
  }

  bool IsConnected() const override {
    return connected_;
  }

  void Close() override {
    close_call_count_++;
    connected_ = false;
  }

  std::string GetLastError() const override {
    if (connect_should_fail_) return connect_error_;
    if (send_should_fail_) return send_error_;
    if (receive_should_fail_) return receive_error_;
    return "";
  }

 private:
  std::queue<std::string> responses_;
  std::vector<std::string> sent_messages_;
  std::vector<std::string> connect_hosts_;
  std::vector<uint16_t> connect_ports_;
  std::string connect_error_;
  std::string send_error_;
  std::string receive_error_;
  bool connected_;
  bool connect_should_fail_;
  bool send_should_fail_;
  bool receive_should_fail_;
  int connect_call_count_;
  int close_call_count_;
};

}  // namespace ipc
}  // namespace cef_ui
