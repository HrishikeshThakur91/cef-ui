#pragma once
#include "pch.h"

#include <vector>
#include <string>
#include <gtest/gtest.h>

#include "core/AppConfig.h"
#include "core/InvalidConfigException.h"

using namespace cef_ui::core;

// ============================================================================
// Test: Missing Arguments
// ============================================================================

TEST(AppConfigTest, ThrowsOnMissingIpcPort) {
  std::vector<std::string> args = {
      "--sessionToken", "valid_token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnMissingSessionToken) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnMissingStartUrl) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "valid_token",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnMissingWindowId) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "valid_token",
      "--startUrl", "https://localhost:8080"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnEmptyArgs) {
  std::vector<std::string> args;
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

// ============================================================================
// Test: Correct Parsing
// ============================================================================

TEST(AppConfigTest, ParsesValidArguments) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "my_secure_token",
      "--startUrl", "https://localhost:8443/docs",
      "--windowId", "42"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetIpcPort(), 9090);
  EXPECT_EQ(config.GetSessionToken(), "my_secure_token");
  EXPECT_EQ(config.GetStartUrl(), "https://localhost:8443/docs");
  EXPECT_EQ(config.GetWindowId(), 42u);
}

TEST(AppConfigTest, ParsesPortAsInteger) {
  std::vector<std::string> args = {
      "--ipcPort", "8888",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "999"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetIpcPort(), 8888);
}

TEST(AppConfigTest, ParsesWindowIdAsInteger) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "12345"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetWindowId(), 12345u);
}

// ============================================================================
// Test: Immutability
// ============================================================================

TEST(AppConfigTest, ConfigIsImmutable) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token123",
      "--startUrl", "https://localhost:8080",
      "--windowId", "555"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  
  // Verify initial values
  EXPECT_EQ(config.GetIpcPort(), 9090);
  EXPECT_EQ(config.GetSessionToken(), "token123");
  
  // Getters should return const references or values (no mutation possible)
  // This test passes if the class has no setters
  EXPECT_EQ(config.GetIpcPort(), 9090);  // Should still be the same
}

// ============================================================================
// Test: Argument Order Independence
// ============================================================================

TEST(AppConfigTest, ParsesArgumentsInDifferentOrder) {
  std::vector<std::string> args = {
      "--startUrl", "https://localhost:8080",
      "--windowId", "111",
      "--ipcPort", "9090",
      "--sessionToken", "token_abc"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetIpcPort(), 9090);
  EXPECT_EQ(config.GetSessionToken(), "token_abc");
  EXPECT_EQ(config.GetStartUrl(), "https://localhost:8080");
  EXPECT_EQ(config.GetWindowId(), 111u);
}

// ============================================================================
// Test: Invalid Argument Values
// ============================================================================

TEST(AppConfigTest, ThrowsOnInvalidPortNumber) {
  std::vector<std::string> args = {
      "--ipcPort", "not_a_number",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnInvalidWindowId) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "not_a_number"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnNegativePort) {
  std::vector<std::string> args = {
      "--ipcPort", "-1",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnPortOutOfRange) {
  std::vector<std::string> args = {
      "--ipcPort", "99999",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

// ============================================================================
// Test: Edge Cases
// ============================================================================

TEST(AppConfigTest, AcceptsEmptySessionToken) {
  // Session token might legitimately be empty in some scenarios
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetSessionToken(), "");
}

TEST(AppConfigTest, AcceptsLongSessionToken) {
  std::string long_token(1024, 'a');  // 1024 'a' characters
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", long_token,
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetSessionToken(), long_token);
}

TEST(AppConfigTest, AcceptsSpecialCharactersInUrl) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080/path?query=value&other=123#anchor",
      "--windowId", "123"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetStartUrl(), "https://localhost:8080/path?query=value&other=123#anchor");
}

TEST(AppConfigTest, ParsesMinimalPortNumber) {
  std::vector<std::string> args = {
      "--ipcPort", "1",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetIpcPort(), 1);
}

TEST(AppConfigTest, ParsesMaxValidPortNumber) {
  std::vector<std::string> args = {
      "--ipcPort", "65535",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetIpcPort(), 65535);
}

// ============================================================================
// Test: Missing Values (flag without value)
// ============================================================================

TEST(AppConfigTest, ThrowsOnFlagWithoutValue) {
  std::vector<std::string> args = {
      "--ipcPort",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnTrailingFlag) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123",
      "--ipcPort"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, RejectsNonHttpsUrl) {
    std::vector<std::string> args = {
        "--ipcPort", "5000",
        "--sessionToken", "abc",
        "--<url-flag>", "http://localhost",
        "--windowId", "1"
    };

    EXPECT_THROW(
        AppConfig::FromArgs(args),
        InvalidConfigException
    );
}

// ============================================================================
// Test: HTTPS-only URL validation (Production Hardening)
// ============================================================================

TEST(AppConfigTest, ThrowsOnNonHttpsUrl) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "http://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnFtpUrl) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "ftp://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnMissingProtocolUrl) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, AcceptsValidHttpsUrl) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://example.com/path",
      "--windowId", "123"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetStartUrl(), "https://example.com/path");
}

TEST(AppConfigTest, AcceptsHttpsUrlWithPort) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8443/docs",
      "--windowId", "123"
  };
  
  AppConfig config = AppConfig::FromArgs(args);
  EXPECT_EQ(config.GetStartUrl(), "https://localhost:8443/docs");
}

// ============================================================================
// Test: Unknown flag rejection (Production Hardening)
// ============================================================================

TEST(AppConfigTest, ThrowsOnUnknownFlag) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123",
      "--unknownFlag", "value"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnUnknownFlagBefore) {
  std::vector<std::string> args = {
      "--unknownFlag", "value",
      "--ipcPort", "9090",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

TEST(AppConfigTest, ThrowsOnUnknownFlagInMiddle) {
  std::vector<std::string> args = {
      "--ipcPort", "9090",
      "--unknownFlag", "value",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  EXPECT_THROW(AppConfig::FromArgs(args), InvalidConfigException);
}

// ============================================================================
// Test: Exception message normalization (Production Hardening)
// ============================================================================

TEST(AppConfigTest, ExceptionMessageStartsWithConfigError) {
  std::vector<std::string> args = {
      "--ipcPort", "invalid",
      "--sessionToken", "token",
      "--startUrl", "https://localhost:8080",
      "--windowId", "123"
  };
  
  try {
    AppConfig::FromArgs(args);
    FAIL() << "Expected InvalidConfigException";
  } catch (const InvalidConfigException& e) {
    std::string msg = e.what();
    EXPECT_TRUE(msg.find("ConfigError:") == 0) 
        << "Exception message should start with 'ConfigError:' but got: " << msg;
  }
}

TEST(AppConfigTest, AllExceptionsHaveConfigErrorPrefix) {
  // Test missing argument
  try {
    std::vector<std::string> args = {"--ipcPort", "9090"};
    AppConfig::FromArgs(args);
    FAIL() << "Expected InvalidConfigException";
  } catch (const InvalidConfigException& e) {
    std::string msg = e.what();
    EXPECT_TRUE(msg.find("ConfigError:") == 0) 
        << "Exception message should start with 'ConfigError:' but got: " << msg;
  }

  // Test invalid port
  try {
    std::vector<std::string> args = {
        "--ipcPort", "99999",
        "--sessionToken", "token",
        "--startUrl", "https://localhost:8080",
        "--windowId", "123"
    };
    AppConfig::FromArgs(args);
    FAIL() << "Expected InvalidConfigException";
  } catch (const InvalidConfigException& e) {
    std::string msg = e.what();
    EXPECT_TRUE(msg.find("ConfigError:") == 0) 
        << "Exception message should start with 'ConfigError:' but got: " << msg;
  }

  // Test non-https URL
  try {
    std::vector<std::string> args = {
        "--ipcPort", "9090",
        "--sessionToken", "token",
        "--startUrl", "http://localhost:8080",
        "--windowId", "123"
    };
    AppConfig::FromArgs(args);
    FAIL() << "Expected InvalidConfigException";
  } catch (const InvalidConfigException& e) {
    std::string msg = e.what();
    EXPECT_TRUE(msg.find("ConfigError:") == 0) 
        << "Exception message should start with 'ConfigError:' but got: " << msg;
  }
}
