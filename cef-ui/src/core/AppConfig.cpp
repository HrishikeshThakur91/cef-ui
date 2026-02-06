#include "../../inc/core/AppConfig.h"
#include "../../inc/core/InvalidConfigException.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <climits>

namespace cef_ui {
namespace core {

AppConfig::AppConfig(uint16_t ipc_port,
                     const std::string& session_token,
                     const std::string& start_url,
                     uint32_t window_id,
                     const std::string& control_file,
                     const std::string& control_key)
    : ipc_port_(ipc_port),
      session_token_(session_token),
      start_url_(start_url),
      window_id_(window_id),
      control_file_(control_file),
      control_key_(control_key) {}

AppConfig AppConfig::FromArgs(const std::vector<std::string>& args) {
  if (args.empty()) {
    throw InvalidConfigException("ConfigError:  No arguments provided");
  }

  uint16_t ipc_port = 0;
  std::string session_token;
  std::string start_url;
  uint32_t window_id = 0;
  std::string control_file;  // Optional
  std::string control_key;   // Optional

  bool has_ipc_port = false;
  bool has_session_token = false;
  bool has_start_url = false;
  bool has_window_id = false;
  // controlFile and controlKey are optional, no flags needed

  // PHASE 1: Parse all arguments WITHOUT validation (collect values only)
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string& arg = args[i];

    if (arg == "--ipcPort") {
      if (i + 1 >= args.size()) {
        throw InvalidConfigException("ConfigError:  --ipcPort requires a value");
      }
      try {
        int port_val = std::stoi(args[i + 1]);
        ipc_port = static_cast<uint16_t>(port_val);
        has_ipc_port = true;
        ++i;  // Skip the value
      } catch (const std::invalid_argument&) {
        throw InvalidConfigException("ConfigError:  --ipcPort value is not a valid integer");
      } catch (const std::out_of_range&) {
        throw InvalidConfigException("ConfigError:  --ipcPort value is out of range");
      }
    } else if (arg == "--sessionToken") {
      if (i + 1 >= args.size()) {
        throw InvalidConfigException("ConfigError:  --sessionToken requires a value");
      }
      session_token = args[i + 1];
      has_session_token = true;
      ++i;  // Skip the value
    } else if (arg == "--startUrl") {
      if (i + 1 >= args.size()) {
        throw InvalidConfigException("ConfigError:  --startUrl requires a value");
      }
      start_url = args[i + 1];
      has_start_url = true;
      ++i;  // Skip the value
    } else if (arg == "--windowId") {
      if (i + 1 >= args.size()) {
        throw InvalidConfigException("ConfigError:  --windowId requires a value");
      }
      try {
        long window_id_val = std::stol(args[i + 1]);
        window_id = static_cast<uint32_t>(window_id_val);
        has_window_id = true;
        ++i;  // Skip the value
      } catch (const std::invalid_argument&) {
        throw InvalidConfigException("ConfigError:  --windowId value is not a valid integer");
      } catch (const std::out_of_range&) {
        throw InvalidConfigException("ConfigError:  --windowId value is out of range");
      }
    } else if (arg == "--controlFile") {
      if (i + 1 >= args.size()) {
        throw InvalidConfigException("ConfigError:  --controlFile requires a value");
      }
      control_file = args[i + 1];
      ++i;  // Skip the value
    } else if (arg == "--controlKey") {
      if (i + 1 >= args.size()) {
        throw InvalidConfigException("ConfigError:  --controlKey requires a value");
      }
      control_key = args[i + 1];
      ++i;  // Skip the value
    } else {
      throw InvalidConfigException(
          "ConfigError: Unknown argument '" + arg + "'"
      );
    }
  }

  // PHASE 2: Validate all parsed values AFTER iteration completes
  
  // Verify all required arguments are present
  if (!has_ipc_port) {
    throw InvalidConfigException("ConfigError:  Missing required argument: --ipcPort");
  }
  if (!has_session_token) {
    throw InvalidConfigException("ConfigError:  Missing required argument: --sessionToken");
  }
  if (!has_start_url) {
    throw InvalidConfigException("ConfigError:  Missing required argument: --startUrl");
  }
  if (!has_window_id) {
    throw InvalidConfigException("ConfigError:  Missing required argument: --windowId");
  }

  // Validate ipcPort range [1, 65535]
  if (ipc_port < 1 || ipc_port > 65535) {
    throw InvalidConfigException("ConfigError:  --ipcPort must be in range [1, 65535]");
  }

  // Validate windowId range [0, UINT32_MAX]
  if (window_id < 0 || window_id > UINT32_MAX) {
    throw InvalidConfigException("ConfigError:  --windowId is out of valid range");
  }

  // Validate startUrl must start with https://
  if (start_url.rfind("https://", 0) != 0) {
    throw InvalidConfigException(
        "ConfigError: --startUrl must start with https://"
    );
  }

  return AppConfig(ipc_port, session_token, start_url, window_id,
                   control_file, control_key);
}

uint16_t AppConfig::GetIpcPort() const {
  return ipc_port_;
}

const std::string& AppConfig::GetSessionToken() const {
  return session_token_;
}

const std::string& AppConfig::GetStartUrl() const {
  return start_url_;
}

uint32_t AppConfig::GetWindowId() const {
  return window_id_;
}

const std::string& AppConfig::GetControlFile() const {
  return control_file_;
}

const std::string& AppConfig::GetControlKey() const {
  return control_key_;
}

}  // namespace core
}  // namespace cef_ui
