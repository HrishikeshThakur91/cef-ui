#include "../../inc/ui/ShutdownCoordinator.h"

namespace cef_ui {
namespace ui {

ShutdownCoordinator::ShutdownCoordinator() : shutdown_requested_(false) {
}

void ShutdownCoordinator::RequestShutdown() noexcept {
  shutdown_requested_ = true;
}

bool ShutdownCoordinator::IsShutdownRequested() const noexcept {
  return shutdown_requested_;
}

}  // namespace ui
}  // namespace cef_ui
