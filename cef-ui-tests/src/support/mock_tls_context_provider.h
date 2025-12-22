#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ipc/ITlsContextProvider.h"

namespace cef_ui {
namespace ipc {

/// Mock TLS context for unit testing.
class MockTlsContext : public ITlsContext {
 public:
  MockTlsContext() = default;
};

/// Mock TLS context provider for unit testing.
/// Allows testing TLS configuration without OS calls.
class MockTlsContextProvider : public ITlsContextProvider {
 public:
  MockTlsContextProvider() : is_available_(true) {}

  // Test configuration
  void SetAvailable(bool available) {
    is_available_ = available;
  }

  void SetDiagnostics(const std::string& info) {
    diagnostics_ = info;
  }

  // Call tracking
  std::vector<std::string> GetCreatedContextsFor() const {
    return created_for_;
  }

  int GetCreateTlsContextCallCount() const {
    return static_cast<int>(created_for_.size());
  }

  // ITlsContextProvider implementation
  std::unique_ptr<ITlsContext> CreateTlsContext(
      const std::string& host) override {
    created_for_.push_back(host);
    if (!is_available_) {
      throw std::runtime_error("TLS context provider not available");
    }
    return std::make_unique<MockTlsContext>();
  }

  bool IsAvailable() const override {
    return is_available_;
  }

  std::string GetDiagnostics() const override {
    return diagnostics_;
  }

 private:
  bool is_available_;
  std::string diagnostics_;
  std::vector<std::string> created_for_;
};

}  // namespace ipc
}  // namespace cef_ui
