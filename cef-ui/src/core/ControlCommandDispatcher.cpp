#include "../../inc/core/ControlCommandDispatcher.h"
#include "../../inc/core/Logger.h"

namespace cef_ui {
namespace core {

ControlCommandDispatcher::ControlCommandDispatcher(
    ui::BrowserInstance& browser,
    ui::ShutdownCoordinator& shutdown)
    : browser_(browser), shutdown_(shutdown) {
  Logger::info("ControlCommandDispatcher", "Initialized");
}

void ControlCommandDispatcher::OnCommand(const ControlCommand& command) {
  try {
    Logger::info("ControlCommandDispatcher", "Received command: " + command.GetCommandId());

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
        Logger::error("ControlCommandDispatcher", "Unknown command type, ignoring");
        return;  // Explicit return, no fallthrough
    }

  } catch (const std::exception& e) {
    Logger::error("ControlCommandDispatcher", "Error processing command: " + std::string(e.what()));
    // Continue operation despite error
  }
}

void ControlCommandDispatcher::Shutdown() {
  Logger::info("ControlCommandDispatcher", "Shutdown called");
  // No cleanup needed (references don't own resources)
}

void ControlCommandDispatcher::HandleStart(const ControlCommand& command) {
  Logger::info("ControlCommandDispatcher", "START command received (no-op): " + command.GetCommandId());
  // No-op as per requirements
}

void ControlCommandDispatcher::HandleNavigate(const ControlCommand& command) {
  Logger::info("ControlCommandDispatcher", "NAVIGATE command received: " + command.GetCommandId());

  // Extract URL from payload
  const auto& payload = command.GetPayload();
  auto it = payload.find("url");

  // Validate URL presence
  if (it == payload.end()) {
    Logger::warn("ControlCommandDispatcher", "NAVIGATE command missing 'url' in payload");
    return;
  }

  const std::string& url = it->second;

  // Validate URL is non-empty
  if (url.empty()) {
    Logger::warn("ControlCommandDispatcher", "NAVIGATE command has empty 'url' in payload");
    return;
  }

  Logger::info("ControlCommandDispatcher", "Navigating to: " + url);

  try {
    // Load URL on browser instance
    // NOTE: This call may need to be marshalled to the CEF UI thread by the integration layer
    // when real CEF is integrated. This class does NOT perform thread marshalling.
    browser_.LoadUrl(url);
    Logger::info("ControlCommandDispatcher", "Navigation initiated successfully");

  } catch (const std::exception& e) {
    Logger::error("ControlCommandDispatcher", "Navigation failed: " + std::string(e.what()));
  }
}

void ControlCommandDispatcher::HandleShutdown(const ControlCommand& command) {
  Logger::info("ControlCommandDispatcher", "SHUTDOWN command received: " + command.GetCommandId());

  // Check if shutdown already requested (idempotency)
  if (shutdown_requested_) {
    Logger::warn("ControlCommandDispatcher", "Shutdown already requested, ignoring duplicate");
    return;
  }

  shutdown_requested_ = true;

  // Request graceful shutdown
  shutdown_.RequestShutdown();
  Logger::info("ControlCommandDispatcher", "Shutdown requested");
}

void ControlCommandDispatcher::HandleHealthPing(const ControlCommand& command) {
  Logger::info("ControlCommandDispatcher", "HEALTH_PING command received (no-op): " + command.GetCommandId());
  // No-op as per requirements
}

}  // namespace core
}  // namespace cef_ui
