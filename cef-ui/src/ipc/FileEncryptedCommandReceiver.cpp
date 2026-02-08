#include "../../inc/ipc/FileEncryptedCommandReceiver.h"
#include "../../inc/ipc/JsonParser.h"
#include "../../inc/core/Logger.h"
#include <fstream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <atomic>
#include <windows.h>
#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

namespace cef_ui {
namespace ipc {

namespace {

// Base64 decoding table
constexpr char kBase64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// AES-256-GCM constants
constexpr size_t kAesKeySize = 32;      // 256 bits
constexpr size_t kGcmIvSize = 12;       // 12 bytes
constexpr size_t kGcmTagSize = 16;      // 16 bytes (128 bits)
constexpr size_t kMinEncryptedSize = kGcmIvSize + kGcmTagSize;

// Polling interval
constexpr int kPollingIntervalMs = 100;

}  // namespace

FileEncryptedCommandReceiver::FileEncryptedCommandReceiver(
    std::filesystem::path controlFile,
    std::string base64Key,
    core::IControlCommandReceiver& receiver)
    : control_file_(std::move(controlFile)),
      receiver_(receiver),
      last_modified_(),
      running_(false) {
  
  // Decode and validate Base64 key
  aes_key_ = DecodeBase64(base64Key);
  
  if (aes_key_.size() != kAesKeySize) {
    throw std::invalid_argument(
        "Invalid key length: expected 32 bytes, got " + 
        std::to_string(aes_key_.size()));
  }

  Logger::info("FileEncryptedCommandReceiver", "Initialized with file: " + control_file_.string());
}

FileEncryptedCommandReceiver::~FileEncryptedCommandReceiver() {
  Stop();
}

void FileEncryptedCommandReceiver::Start() {
  if (running_) {
    Logger::warn("FileEncryptedCommandReceiver", "Already running");
    return;
  }

  running_ = true;
  Logger::info("FileEncryptedCommandReceiver", "Starting polling thread...");

  // Start polling in background thread
  polling_thread_ = std::thread(&FileEncryptedCommandReceiver::PollingLoop, this);

  Logger::info("FileEncryptedCommandReceiver", "Polling thread started");
}

void FileEncryptedCommandReceiver::Stop() {
  if (!running_) {
    return;
  }

  Logger::info("FileEncryptedCommandReceiver", "Stopping polling...");
  running_ = false;

  // Wait for polling thread to finish
  if (polling_thread_.joinable()) {
    polling_thread_.join();
  }

  Logger::info("FileEncryptedCommandReceiver", "Polling stopped");
}

std::vector<uint8_t> FileEncryptedCommandReceiver::DecodeBase64(
    const std::string& base64) {
  std::vector<uint8_t> result;
  
  // Build reverse lookup table
  int decode_table[256];
  std::fill(std::begin(decode_table), std::end(decode_table), -1);
  for (int i = 0; i < 64; ++i) {
    decode_table[static_cast<unsigned char>(kBase64Chars[i])] = i;
  }

  size_t i = 0;
  while (i < base64.length()) {
    // Skip whitespace and padding
    if (base64[i] == '=' || std::isspace(base64[i])) {
      ++i;
      continue;
    }

    // Collect 4 base64 characters
    int values[4] = {-1, -1, -1, -1};
    int count = 0;
    
    while (count < 4 && i < base64.length()) {
      if (base64[i] == '=' || std::isspace(base64[i])) {
        ++i;
        continue;
      }
      
      int val = decode_table[static_cast<unsigned char>(base64[i])];
      if (val == -1) {
        throw std::invalid_argument("Invalid Base64 character");
      }
      
      values[count++] = val;
      ++i;
    }

    // Decode to bytes
    if (count >= 2) {
      result.push_back((values[0] << 2) | (values[1] >> 4));
    }
    if (count >= 3) {
      result.push_back(((values[1] & 0x0F) << 4) | (values[2] >> 2));
    }
    if (count >= 4) {
      result.push_back(((values[2] & 0x03) << 6) | values[3]);
    }
  }

  return result;
}

bool FileEncryptedCommandReceiver::HasFileChanged() {
  if (!std::filesystem::exists(control_file_)) {
    return false;
  }

  try {
    auto current_modified = std::filesystem::last_write_time(control_file_);
    
    if (current_modified != last_modified_) {
      last_modified_ = current_modified;
      return true;
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "[FileEncryptedCommandReceiver] Error checking file time: "
              << e.what() << std::endl;
  }

  return false;
}

std::vector<uint8_t> FileEncryptedCommandReceiver::ReadFile() {
  std::ifstream file(control_file_, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to open control file");
  }

  // Read entire file
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char*>(data.data()), size);

  if (!file) {
    throw std::runtime_error("Failed to read control file");
  }

  return data;
}

std::string FileEncryptedCommandReceiver::Decrypt(
    const std::vector<uint8_t>& encrypted) {
  
  if (encrypted.size() < kMinEncryptedSize) {
    throw std::runtime_error(
        "Encrypted data too small: " + std::to_string(encrypted.size()) +
        " bytes (minimum " + std::to_string(kMinEncryptedSize) + ")");
  }

  // Extract IV (first 12 bytes)
  std::vector<uint8_t> iv(encrypted.begin(), encrypted.begin() + kGcmIvSize);

  // Extract ciphertext + tag (remaining bytes)
  std::vector<uint8_t> ciphertext_with_tag(
      encrypted.begin() + kGcmIvSize, encrypted.end());

  // Windows CryptoAPI decryption
  BCRYPT_ALG_HANDLE hAlg = nullptr;
  BCRYPT_KEY_HANDLE hKey = nullptr;
  NTSTATUS status;

  try {
    // Open algorithm provider
    status = BCryptOpenAlgorithmProvider(
        &hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (!BCRYPT_SUCCESS(status)) {
      throw std::runtime_error("Failed to open AES algorithm provider");
    }

    // Set GCM mode
    status = BCryptSetProperty(
        hAlg, BCRYPT_CHAINING_MODE,
        reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(BCRYPT_CHAIN_MODE_GCM)),
        sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (!BCRYPT_SUCCESS(status)) {
      BCryptCloseAlgorithmProvider(hAlg, 0);
      throw std::runtime_error("Failed to set GCM mode");
    }

    // Generate key object
    status = BCryptGenerateSymmetricKey(
        hAlg, &hKey,
        nullptr, 0,  // No key object buffer needed
        const_cast<PUCHAR>(aes_key_.data()), kAesKeySize, 0);
    if (!BCRYPT_SUCCESS(status)) {
      BCryptCloseAlgorithmProvider(hAlg, 0);
      throw std::runtime_error("Failed to generate symmetric key");
    }

    // Setup GCM authenticated encryption info
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = iv.data();
    authInfo.cbNonce = kGcmIvSize;
    authInfo.pbTag = ciphertext_with_tag.data() + ciphertext_with_tag.size() - kGcmTagSize;
    authInfo.cbTag = kGcmTagSize;

    // Calculate plaintext size (ciphertext without tag)
    ULONG plaintextSize = ciphertext_with_tag.size() - kGcmTagSize;
    std::vector<uint8_t> plaintext(plaintextSize);

    ULONG bytesDecrypted = 0;
    status = BCryptDecrypt(
        hKey,
        ciphertext_with_tag.data(), plaintextSize,
        &authInfo,
        nullptr, 0,  // No IV buffer needed (in authInfo)
        plaintext.data(), plaintextSize,
        &bytesDecrypted, 0);

    // Cleanup
    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    if (!BCRYPT_SUCCESS(status)) {
      throw std::runtime_error("Decryption failed (authentication failed or invalid data)");
    }

    // Convert to string
    return std::string(plaintext.begin(), plaintext.begin() + bytesDecrypted);

  } catch (...) {
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    throw;
  }
}

core::ControlCommand FileEncryptedCommandReceiver::ParseCommand(
    const std::string& json) {
  
  // Parse JSON
  auto root = SimpleJsonParser::Parse(json);
  if (!root || root->type != JsonValue::OBJECT) {
    throw std::runtime_error("Invalid JSON: root is not an object");
  }

  // Extract commandId
  std::string command_id;
  if (!SimpleJsonParser::TryGetStringValue(root.get(), "commandId", command_id)) {
    throw std::runtime_error("Missing or invalid field: commandId");
  }

  // Extract type
  std::string type_str;
  if (!SimpleJsonParser::TryGetStringValue(root.get(), "type", type_str)) {
    throw std::runtime_error("Missing or invalid field: type");
  }

  // Convert type string to enum
  // Java writes enum.name() which produces: START, NAVIGATE, SHUTDOWN, HEALTH_PING
  core::ControlCommandType type;
  if (type_str == "START") {
    type = core::ControlCommandType::Start;
  } else if (type_str == "NAVIGATE") {
    type = core::ControlCommandType::Navigate;
  } else if (type_str == "SHUTDOWN") {
    type = core::ControlCommandType::Shutdown;
  } else if (type_str == "HEALTH_PING") {
    type = core::ControlCommandType::HealthPing;
  } else {
    throw std::runtime_error("Unknown command type: " + type_str);
  }

  // Extract payload (optional)
  std::map<std::string, std::string> payload;
  for (const auto& pair : root->object_value) {
    if (pair.first == "payload" && pair.second && 
        pair.second->type == JsonValue::OBJECT) {
      for (const auto& payload_pair : pair.second->object_value) {
        if (payload_pair.second && 
            payload_pair.second->type == JsonValue::STRING) {
          payload[payload_pair.first] = payload_pair.second->string_value;
        }
      }
    }
  }

  // Extract timestamp
  auto timestamp = std::chrono::system_clock::now();
  for (const auto& pair : root->object_value) {
    if (pair.first == "timestamp" && pair.second && 
        pair.second->type == JsonValue::NUMBER) {
      auto millis = static_cast<long long>(pair.second->number_value);
      timestamp = std::chrono::system_clock::time_point(
          std::chrono::milliseconds(millis));
    }
  }

  return core::ControlCommand(command_id, type, payload, timestamp);
}

void FileEncryptedCommandReceiver::ProcessFile() {
  try {
    // Read encrypted file
    auto encrypted = ReadFile();
    Logger::info("FileEncryptedCommandReceiver", "Read " + std::to_string(encrypted.size()) + " bytes from file");

    // Decrypt
    std::string json = Decrypt(encrypted);
    Logger::info("FileEncryptedCommandReceiver", "Decrypted JSON: " + json);

    // Parse command
    auto command = ParseCommand(json);
    Logger::info("FileEncryptedCommandReceiver", "Parsed command: " + command.GetCommandId());

    // Forward to receiver
    receiver_.OnCommand(command);
    Logger::info("FileEncryptedCommandReceiver", "Command forwarded successfully");

  } catch (const std::exception& e) {
    Logger::error("FileEncryptedCommandReceiver", "Failed to process file: " + std::string(e.what()));
    // Continue polling despite error
  }
}

void FileEncryptedCommandReceiver::PollingLoop() {
  Logger::info("FileEncryptedCommandReceiver", "Polling loop started");

  while (running_) {
    try {
      if (HasFileChanged()) {
        ProcessFile();
      }
    } catch (const std::exception& e) {
      std::cerr << "[FileEncryptedCommandReceiver] Error in polling loop: "
                << e.what() << std::endl;
    }

    // Sleep to avoid busy-waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(kPollingIntervalMs));
  }

  Logger::info("FileEncryptedCommandReceiver", "Polling loop finished");
}

}  // namespace ipc
}  // namespace cef_ui
