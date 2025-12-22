#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ipc/WssConnectionManager.h"

namespace cef_ui {
namespace ipc {

/// Mock connection listener for unit testing.
/// Tracks lifecycle events in order while implementing IConnectionListener interface.
class MockConnectionListener : public IConnectionListener {
 public:
  MockConnectionListener() : on_connecting_count_(0), on_connected_count_(0),
                             on_disconnected_count_(0), on_error_count_(0) {}

  // Call tracking
  std::vector<std::string> GetEventSequence() const {
    return event_sequence_;
  }

  int GetOnConnectingCallCount() const {
    return on_connecting_count_;
  }

  int GetOnConnectedCallCount() const {
    return on_connected_count_;
  }

  int GetOnDisconnectedCallCount() const {
    return on_disconnected_count_;
  }

  int GetOnErrorCallCount() const {
    return on_error_count_;
  }

  std::vector<std::string> GetErrorMessages() const {
    return error_messages_;
  }

  // IConnectionListener implementation
  void OnConnecting() override {
    on_connecting_count_++;
    event_sequence_.push_back("OnConnecting");
  }

  void OnConnected() override {
    on_connected_count_++;
    event_sequence_.push_back("OnConnected");
  }

  void OnDisconnected() override {
    on_disconnected_count_++;
    event_sequence_.push_back("OnDisconnected");
  }

  void OnError(const std::string& error_msg) override {
    on_error_count_++;
    error_messages_.push_back(error_msg);
    event_sequence_.push_back("OnError:" + error_msg);
  }

 private:
  std::vector<std::string> event_sequence_;
  std::vector<std::string> error_messages_;
  int on_connecting_count_;
  int on_connected_count_;
  int on_disconnected_count_;
  int on_error_count_;
};

}  // namespace ipc
}  // namespace cef_ui
