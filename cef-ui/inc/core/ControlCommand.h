#pragma once

#include <chrono>
#include <map>
#include <stdexcept>
#include <string>
#include "ControlCommandType.h"

namespace cef_ui {
namespace core {

/// Immutable control command value object.
/// Represents a command to be sent to the CEF process.
/// All fields are const to ensure immutability.
/// Timestamp is set automatically at construction time and is not used for logic.
/// RAII-safe: Uses standard library types with automatic memory management.
class ControlCommand {
 public:
  /// Creates a new immutable ControlCommand.
  /// Timestamp is set automatically to the current time.
  /// @param command_id Unique identifier for this command (must be non-empty)
  /// @param type The command type
  /// @param payload Optional key-value payload data
  /// @throws std::invalid_argument if command_id is empty
  ControlCommand(std::string command_id,
                 ControlCommandType type,
                 std::map<std::string, std::string> payload)
      : command_id_(std::move(command_id)),
        type_(type),
        payload_(std::move(payload)),
        timestamp_(std::chrono::system_clock::now()) {
    if (command_id_.empty()) {
      throw std::invalid_argument("commandId cannot be empty");
    }
  }

  /// Gets the command ID.
  /// @return The unique command identifier
  const std::string& GetCommandId() const { return command_id_; }

  /// Gets the command type.
  /// @return The command type
  ControlCommandType GetType() const { return type_; }

  /// Gets the payload.
  /// @return A const reference to the payload map
  const std::map<std::string, std::string>& GetPayload() const {
    return payload_;
  }

  /// Gets the timestamp (opaque metadata, not used for logic).
  /// @return The timestamp when this command was created
  std::chrono::system_clock::time_point GetTimestamp() const {
    return timestamp_;
  }

 private:
  const std::string command_id_;
  const ControlCommandType type_;
  const std::map<std::string, std::string> payload_;
  const std::chrono::system_clock::time_point timestamp_;
};

}  // namespace core
}  // namespace cef_ui
