#include "../../inc/ui/CefBrowserManager.h"
#include "../../inc/ui/NativeWindow.h"
#include "../../inc/ui/CefHandlers.h"
#include <stdexcept>
#include <sstream>

namespace cef_ui {
namespace ui {

CefBrowserManager::CefBrowserManager(NativeWindow& window, const std::string& url)
    : window_(window),
      url_(url),
      client_(std::make_unique<CefClientHandler>(&window)),
      browser_(nullptr),
      is_ready_(false) {
  
  if (url.empty()) {
    throw std::runtime_error("URL cannot be empty");
  }

  if (!window.GetHandle()) {
    throw std::runtime_error("Window handle is invalid");
  }

  try {
    CreateBrowser();
  } catch (...) {
    DestroyBrowser();
    throw;
  }
}

CefBrowserManager::~CefBrowserManager() noexcept {
  DestroyBrowser();
}

bool CefBrowserManager::IsReady() const {
  return is_ready_ && browser_ != nullptr;
}

void CefBrowserManager::Close() noexcept {
  if (browser_) {
    // Browser will close asynchronously
    // (actual CEF close call will be added in Phase 6)
  }
}

void CefBrowserManager::CreateBrowser() {
  // Browser creation will be implemented when CEF binaries are available
  // For now, mark as ready to allow testing of the structure
  
  if (!url_.empty() && window_.GetHandle()) {
    is_ready_ = true;
  } else {
    throw std::runtime_error("Failed to create browser: invalid parameters");
  }
}

void CefBrowserManager::DestroyBrowser() noexcept {
  if (browser_) {
    // Browser cleanup will be implemented when CEF binaries are available
    browser_ = nullptr;
  }
  is_ready_ = false;
}

}  // namespace ui
}  // namespace cef_ui
