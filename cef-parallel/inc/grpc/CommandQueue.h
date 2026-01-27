#pragma once

#include <queue>
#include <mutex>
#include <optional>
#include "UiCommand.h"

namespace cef_ui {
namespace grpc_server {

/// Thread-safe queue for storing UI commands
/// Commands are enqueued from gRPC threads and dequeued on CEF UI thread
class CommandQueue {
public:
    CommandQueue() = default;
    ~CommandQueue() = default;
    
    // Non-copyable, non-movable
    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;
    CommandQueue(CommandQueue&&) = delete;
    CommandQueue& operator=(CommandQueue&&) = delete;
    
    /// Enqueue a command (called from gRPC threads)
    void Enqueue(UiCommand cmd) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(cmd));
    }
    
    /// Dequeue a command (called from CEF UI thread)
    /// Returns std::nullopt if queue is empty
    std::optional<UiCommand> Dequeue() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        UiCommand cmd = std::move(queue_.front());
        queue_.pop();
        return cmd;
    }
    
    /// Check if queue is empty
    bool IsEmpty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    /// Get queue size
    size_t Size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::queue<UiCommand> queue_;
};

}  // namespace grpc_server
}  // namespace cef_ui
