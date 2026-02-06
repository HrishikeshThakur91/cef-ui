#include "../../inc/core/ControlCommandDispatcher.h"
#include <iostream>

namespace cef_ui {
namespace core {

ControlCommandDispatcher::ControlCommandDispatcher(
    ui::BrowserInstance& browser,
    ui::ShutdownCoordinator& shutdown)
    : browser_(browser), shutdown_(shutdown) {
  std::cerr << "[ControlCommandDispatcher] Initialized" << std::endl;
}

void ControlCommandDispatcher::OnCommand(const ControlCommand& command) {
  try {
    std::cerr << "[ControlCommandDispatcher] Received command: "
              << command.GetCommandId() << std::endl;

    // Dispatch based on command type
    switch (command.GetType()) {
      case ControlCommandType::Start:
        HandleStart(command);
        break;

      case ControlCommandType::Navigate:
        HandleNavigate(command);
        break;

      case ControlCommandType::Shutdown:
        HandleShutdown(command);
        break;

      case ControlCommandType::HealthPing:
        HandleHealthPing(command);
        break;

      default:
        std::cerr << "[ControlCommandDispatcher] ERROR: Unknown command type, ignoring"
                  << std::endl;
        return;  // Explicit return, no fallthrough
    }

  } catch (const std::exception& e) {
    std::cerr << "[ControlCommandDispatcher] Error processing command: "
              << e.what() << std::endl;
    // Continue operation despite error
  }
}

void ControlCommandDispatcher::Shutdown() {
  std::cerr << "[ControlCommandDispatcher] Shutdown called" << std::endl;
  // No cleanup needed (references don't own resources)
}

void ControlCommandDispatcher::HandleStart(const ControlCommand& command) {
  std::cerr << "[ControlCommandDispatcher] START command received (no-op): "
            << command.GetCommandId() << std::endl;
  // No-op as per requirements
}

void ControlCommandDispatcher::HandleNavigate(const ControlCommand& command) {
  std::cerr << "[ControlCommandDispatcher] NAVIGATE command received: "
            << command.GetCommandId() << std::endl;

  // Extract URL from payload
  const auto& payload = command.GetPayload();
  auto it = payload.find("url");

  // Validate URL presence
  if (it == payload.end()) {
    std::cerr << "[ControlCommandDispatcher] ERROR: NAVIGATE command missing 'url' in payload"
              << std::endl;
    return;
  }

  const std::string& url = it->second;

  // Validate URL is non-empty
  if (url.empty()) {
    std::cerr << "[ControlCommandDispatcher] ERROR: NAVIGATE command has empty 'url' in payload"
              << std::endl;
    return;
  }

  std::cerr << "[ControlCommandDispatcher] Navigating to: " << url << std::endl;

  try {
    // Load URL on browser instance
    // NOTE: This call may need to be marshalled to the CEF UI thread by the integration layer
    // when real CEF is integrated. This class does NOT perform thread marshalling.
    browser_.LoadUrl(url);
    std::cerr << "[ControlCommandDispatcher] Navigation initiated successfully"
              << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "[ControlCommandDispatcher] Navigation failed: " << e.what()
              << std::endl;
  }
}

void ControlCommandDispatcher::HandleShutdown(const ControlCommand& command) {
  std::cerr << "[ControlCommandDispatcher] SHUTDOWN command received: "
            << command.GetCommandId() << std::endl;

  // Check if shutdown already requested (idempotency)
  if (shutdown_requested_) {
    std::cerr << "[ControlCommandDispatcher] Shutdown already requested, ignoring duplicate"
              << std::endl;
    return;
  }

  shutdown_requested_ = true;

  // Request graceful shutdown
  shutdown_.RequestShutdown();
  std::cerr << "[ControlCommandDispatcher] Shutdown requested" << std::endl;
}

void ControlCommandDispatcher::HandleHealthPing(const ControlCommand& command) {
  std::cerr << "[ControlCommandDispatcher] HEALTH_PING command received (no-op): "
            << command.GetCommandId() << std::endl;
  // No-op as per requirements
}

}  // namespace core
}  // namespace cef_ui
