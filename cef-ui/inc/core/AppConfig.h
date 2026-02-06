#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace cef_ui {
namespace core {

/// Immutable application configuration parsed from command-line arguments.
/// 
/// Parses and validates required configuration parameters:
/// - --ipcPort: WebSocket server port (1-65535)
/// - --sessionToken: Session authentication token
/// - --startUrl: Initial URL to load (HTTPS only)
/// - --windowId: Native window identifier
///
/// All parameters are required. Configuration is immutable after creation.
class AppConfig {
 public:
  /// Parse configuration from command-line arguments.
  /// 
  /// @param args Vector of command-line arguments in format: --key value --key value
  /// @return Parsed AppConfig object
  /// @throws InvalidConfigException if parsing fails or required arguments are missing
  static AppConfig FromArgs(const std::vector<std::string>& args);

  // Copy constructor (move semantics for efficiency)
  AppConfig(const AppConfig&) = default;
  AppConfig& operator=(const AppConfig&) = default;
  AppConfig(AppConfig&&) = default;
  AppConfig& operator=(AppConfig&&) = default;

  ~AppConfig() = default;

  /// Get the IPC port number.
  /// @return Port number in range [1, 65535]
  uint16_t GetIpcPort() const;

  /// Get the session token.
  /// @return Session token string (may be empty)
  const std::string& GetSessionToken() const;

  /// Get the initial URL to load.
  /// @return URL string (HTTPS)
  const std::string& GetStartUrl() const;

  /// Get the window ID.
  /// @return Window identifier as unsigned 32-bit integer
  uint32_t GetWindowId() const;

  /// Get the control file path (optional).
  /// @return Control file path, empty if not specified
  const std::string& GetControlFile() const;

  /// Get the control key (optional).
  /// @return Base64-encoded control key, empty if not specified
  const std::string& GetControlKey() const;

 private:
  // Private constructor - only FromArgs can create instances
  AppConfig(uint16_t ipc_port,
            const std::string& session_token,
            const std::string& start_url,
            uint32_t window_id,
            const std::string& control_file,
            const std::string& control_key);

  uint16_t ipc_port_;
  std::string session_token_;
  std::string start_url_;
  uint32_t window_id_;
  std::string control_file_;  // Optional
  std::string control_key_;   // Optional
};

}  // namespace core
}  // namespace cef_ui
