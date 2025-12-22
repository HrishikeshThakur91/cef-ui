#pragma once

#include <stdexcept>
#include <string>

namespace cef_ui {
namespace core {

/// Exception thrown when configuration parsing fails.
/// Derives from std::runtime_error to provide standard exception interface.
class InvalidConfigException : public std::runtime_error {
 public:
  explicit InvalidConfigException(const std::string& message)
      : std::runtime_error(message) {}

  explicit InvalidConfigException(const char* message)
      : std::runtime_error(message) {}
};

}  // namespace core
}  // namespace cef_ui
