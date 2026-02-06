#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include "../core/ControlCommand.h"
#include "../core/IControlCommandReceiver.h"

namespace cef_ui {
namespace ipc {

/// File-based encrypted command receiver.
/// 
/// Polls an encrypted control file for changes, decrypts AES-256-GCM content,
/// parses JSON, and forwards commands to the receiver.
/// 
/// Constraints:
/// - AES-256-GCM only
/// - No CEF includes
/// - No UI calls
/// - No IPC
/// - No sockets
/// - Polling-based
/// - Deterministic
/// - Exception-safe
class FileEncryptedCommandReceiver {
 public:
  /// Creates a new FileEncryptedCommandReceiver.
  /// @param controlFile Path to the encrypted control file
  /// @param base64Key Base64-encoded AES-256 key (must decode to 32 bytes)
  /// @param receiver Receiver to forward commands to
  /// @throws std::invalid_argument if base64Key is invalid or not 32 bytes
  FileEncryptedCommandReceiver(
      std::filesystem::path controlFile,
      std::string base64Key,
      core::IControlCommandReceiver& receiver);

  ~FileEncryptedCommandReceiver();

  /// Starts polling for file changes.
  /// Returns immediately. Polling runs in a background thread.
  void Start();

  /// Stops polling.
  void Stop();

 private:
  std::filesystem::path control_file_;
  std::vector<uint8_t> aes_key_;
  core::IControlCommandReceiver& receiver_;
  std::filesystem::file_time_type last_modified_;
  std::atomic<bool> running_;
  std::thread polling_thread_;

  /// Decodes Base64 string to binary.
  /// @param base64 Base64-encoded string
  /// @return Decoded binary data
  /// @throws std::invalid_argument if base64 is invalid
  std::vector<uint8_t> DecodeBase64(const std::string& base64);

  /// Checks if the control file has been modified.
  /// @return true if file was modified since last check
  bool HasFileChanged();

  /// Reads the entire control file.
  /// @return File contents as binary data
  std::vector<uint8_t> ReadFile();

  /// Decrypts AES-256-GCM encrypted data.
  /// Format: [12-byte IV][ciphertext][16-byte auth tag]
  /// @param encrypted Encrypted data
  /// @return Decrypted plaintext
  /// @throws std::runtime_error if decryption fails
  std::string Decrypt(const std::vector<uint8_t>& encrypted);

  /// Parses JSON and creates ControlCommand.
  /// @param json JSON string
  /// @return Parsed command
  /// @throws std::runtime_error if parsing fails
  core::ControlCommand ParseCommand(const std::string& json);

  /// Processes the control file (read, decrypt, parse, forward).
  /// Catches all exceptions and logs errors.
  void ProcessFile();

  /// Internal polling loop (runs in background thread).
  void PollingLoop();
};

}  // namespace ipc
}  // namespace cef_ui
