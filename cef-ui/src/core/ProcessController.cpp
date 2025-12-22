#include "../../inc/core/ProcessController.h"
#include <algorithm>

namespace cef_ui {
namespace core {

ProcessController::ProcessController()
    : state_(ProcessState::Idle),
      listeners_() {}

void ProcessController::Start() {
  if (state_ != ProcessState::Idle) {
    throw std::logic_error("ProcessController::Start() called but process is not Idle");
  }

  state_ = ProcessState::Starting;
  state_ = ProcessState::Started;
  EmitStarted();
}

void ProcessController::Shutdown() {
  // Idempotent: shutdown is always safe, even if already stopped
  if (state_ == ProcessState::Stopped) {
    return;
  }

  // Only emit events if we're actually shutting down from a running state
  if (state_ == ProcessState::Started) {
    state_ = ProcessState::Stopping;
    EmitStopping();
    state_ = ProcessState::Stopped;
    EmitStopped();
  } else {
    // If in Idle or other states, just transition to Stopped without events
    state_ = ProcessState::Stopped;
  }
}

ProcessState ProcessController::GetState() const {
  return state_;
}

void ProcessController::AddListener(std::shared_ptr<ILifecycleListener> listener) {
  if (listener) {
    listeners_.push_back(listener);
  }
}

void ProcessController::RemoveListener(std::shared_ptr<ILifecycleListener> listener) {
  listeners_.erase(
      std::remove(listeners_.begin(), listeners_.end(), listener),
      listeners_.end());
}

void ProcessController::EmitStarted() {
  for (auto& listener : listeners_) {
    listener->OnStarted();
  }
}

void ProcessController::EmitStopping() {
  for (auto& listener : listeners_) {
    listener->OnStopping();
  }
}

void ProcessController::EmitStopped() {
  for (auto& listener : listeners_) {
    listener->OnStopped();
  }
}

void ProcessController::EmitError(const std::string& error_message) {
  for (auto& listener : listeners_) {
    listener->OnError(error_message);
  }
}

}  // namespace core
}  // namespace cef_ui
