#pragma once
#include "pch.h"

#include <memory>
#include <string>
#include <gtest/gtest.h>

#include "ipc/Handshake.h"
#include "ipc/MessageTypes.h"
#include "ipc/IMessageChannel.h"

using namespace cef_ui::ipc;

// ============================================================================
// Test: MessageParser - Valid HELLO Messages
// ============================================================================

TEST(MessageParserTest, ParseValidHelloMessage) {
  MessageParser parser;
  std::string json = R"({"type":"HELLO","sessionToken":"valid_token_123"})";
  
  auto message = parser.Parse(json);
  
  ASSERT_EQ(message->GetType(), MessageType::HELLO);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  ASSERT_NE(hello, nullptr);
  EXPECT_EQ(hello->GetSessionToken(), "valid_token_123");
}

TEST(MessageParserTest, ParseHelloWithWhitespace) {
  MessageParser parser;
  std::string json = R"({
    "type": "HELLO",
    "sessionToken": "token_abc"
  })";
  
  auto message = parser.Parse(json);
  
  ASSERT_EQ(message->GetType(), MessageType::HELLO);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  EXPECT_EQ(hello->GetSessionToken(), "token_abc");
}

TEST(MessageParserTest, ParseHelloIgnoresUnknownFields) {
  MessageParser parser;
  std::string json = R"({
    "type": "HELLO",
    "sessionToken": "token_xyz",
    "unknownField": "should_be_ignored",
    "anotherField": 12345
  })";
  
  auto message = parser.Parse(json);
  
  ASSERT_EQ(message->GetType(), MessageType::HELLO);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  EXPECT_EQ(hello->GetSessionToken(), "token_xyz");
}

TEST(MessageParserTest, ParseHelloWithEmptySessionToken) {
  MessageParser parser;
  std::string json = R"({"type":"HELLO","sessionToken":""})";
  
  auto message = parser.Parse(json);
  
  ASSERT_EQ(message->GetType(), MessageType::HELLO);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  EXPECT_EQ(hello->GetSessionToken(), "");
}

// ============================================================================
// Test: MessageParser - Valid NAVIGATE Messages
// ============================================================================

TEST(MessageParserTest, ParseValidNavigateMessage) {
  MessageParser parser;
  std::string json = R"({"type":"NAVIGATE","url":"/docs/page"})";
  
  auto message = parser.Parse(json);
  
  ASSERT_EQ(message->GetType(), MessageType::NAVIGATE);
  auto nav = dynamic_cast<NavigateMessageImpl*>(message.get());
  ASSERT_NE(nav, nullptr);
  EXPECT_EQ(nav->GetUrl(), "/docs/page");
}

TEST(MessageParserTest, ParseNavigateIgnoresUnknownFields) {
  MessageParser parser;
  std::string json = R"({
    "type": "NAVIGATE",
    "url": "/page",
    "extra": "field"
  })";
  
  auto message = parser.Parse(json);
  
  ASSERT_EQ(message->GetType(), MessageType::NAVIGATE);
  auto nav = dynamic_cast<NavigateMessageImpl*>(message.get());
  EXPECT_EQ(nav->GetUrl(), "/page");
}

// ============================================================================
// Test: MessageParser - Malformed JSON
// ============================================================================

TEST(MessageParserTest, ThrowsOnMalformedJson) {
  MessageParser parser;
  std::string json = R"({invalid json})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(MessageParserTest, ThrowsOnEmptyJson) {
  MessageParser parser;
  std::string json = "";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(MessageParserTest, ThrowsOnNullJson) {
  MessageParser parser;
  std::string json = "null";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(MessageParserTest, ThrowsOnJsonArray) {
  MessageParser parser;
  std::string json = R"([])";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

// ============================================================================
// Test: MessageParser - Missing Required Fields
// ============================================================================

TEST(MessageParserTest, ThrowsOnMissingTypeField) {
  MessageParser parser;
  std::string json = R"({"sessionToken":"token"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(MessageParserTest, ThrowsOnMissingSessionTokenInHello) {
  MessageParser parser;
  std::string json = R"({"type":"HELLO"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(MessageParserTest, ThrowsOnMissingUrlInNavigate) {
  MessageParser parser;
  std::string json = R"({"type":"NAVIGATE"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

// ============================================================================
// Test: MessageParser - Unknown Message Type
// ============================================================================

TEST(MessageParserTest, ThrowsOnUnknownMessageType) {
  MessageParser parser;
  std::string json = R"({"type":"UNKNOWN_TYPE"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(MessageParserTest, ThrowsOnInvalidTypeValue) {
  MessageParser parser;
  std::string json = R"({"type":123})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

// ============================================================================
// Test: Handshake - Valid Token Validation
// ============================================================================

TEST(HandshakeTest, ValidateHelloWithCorrectToken) {
  Handshake handshake("expected_token_123");
  HelloMessageImpl hello("expected_token_123");
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(HandshakeTest, GetExpectedTokenReturnsCorrectValue) {
  Handshake handshake("my_token_456");
  
  EXPECT_EQ(handshake.GetExpectedToken(), "my_token_456");
}

// ============================================================================
// Test: Handshake - Invalid Token Rejection
// ============================================================================

TEST(HandshakeTest, RejectHelloWithIncorrectToken) {
  Handshake handshake("expected_token");
  HelloMessageImpl hello("wrong_token");
  
  EXPECT_THROW(handshake.ValidateHello(hello), IpcProtocolException);
}

TEST(HandshakeTest, RejectHelloWithEmptyTokenWhenExpected) {
  Handshake handshake("expected_token");
  HelloMessageImpl hello("");
  
  EXPECT_THROW(handshake.ValidateHello(hello), IpcProtocolException);
}

TEST(HandshakeTest, RejectHelloWhenExpectedTokenIsEmpty) {
  // Edge case: empty expected token
  Handshake handshake("");
  HelloMessageImpl hello("some_token");
  
  EXPECT_THROW(handshake.ValidateHello(hello), IpcProtocolException);
}

TEST(HandshakeTest, TokenValidationIsCaseSensitive) {
  Handshake handshake("TOKEN");
  HelloMessageImpl hello("token");
  
  EXPECT_THROW(handshake.ValidateHello(hello), IpcProtocolException);
}

// ============================================================================
// Test: Handshake - Token Normalization (Whitespace Trimming)
// ============================================================================

TEST(HandshakeTest, ValidateHelloTrimsLeadingWhitespace) {
  // Token with leading whitespace in message should match expected token
  Handshake handshake("expected_token");
  HelloMessageImpl hello("  expected_token");  // Leading spaces
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(HandshakeTest, ValidateHelloTrimsTrailingWhitespace) {
  // Token with trailing whitespace in message should match expected token
  Handshake handshake("expected_token");
  HelloMessageImpl hello("expected_token  ");  // Trailing spaces
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(HandshakeTest, ValidateHelloTrimsLeadingAndTrailingWhitespace) {
  // Token with both leading and trailing whitespace
  Handshake handshake("expected_token");
  HelloMessageImpl hello("  expected_token  ");
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(HandshakeTest, ValidateHelloTrimsTabsAndNewlines) {
  // Token with tabs and newlines
  Handshake handshake("expected_token");
  HelloMessageImpl hello("\t\nexpected_token\n\t");
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(HandshakeTest, ValidateHelloPreservesInternalWhitespace) {
  // Internal whitespace in token should be preserved
  Handshake handshake("token with spaces");
  HelloMessageImpl hello("  token with spaces  ");
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(HandshakeTest, ValidateHelloRejectsWrongTokenAfterTrim) {
  // Even with trimming, wrong token should still be rejected
  Handshake handshake("expected_token");
  HelloMessageImpl hello("  wrong_token  ");
  
  EXPECT_THROW(handshake.ValidateHello(hello), IpcProtocolException);
}

TEST(HandshakeTest, ValidateHelloEmptyTokenAfterTrim) {
  // Token that is only whitespace after trim should be treated as empty
  Handshake handshake("expected_token");
  HelloMessageImpl hello("   ");  // Only whitespace
  
  EXPECT_THROW(handshake.ValidateHello(hello), IpcProtocolException);
}

TEST(HandshakeTest, ProcessHelloMessageTrimsTokenInJson) {
  Handshake handshake("correct_token");
  std::string json = R"({"type":"HELLO","sessionToken":"  correct_token  "})";
  
  EXPECT_NO_THROW(handshake.ProcessHelloMessage(json));
}

// ============================================================================
// Test: Handshake - Process HELLO Message (JSON to Validation)
// ============================================================================

TEST(HandshakeTest, ProcessValidHelloMessage) {
  Handshake handshake("correct_token");
  std::string json = R"({"type":"HELLO","sessionToken":"correct_token"})";
  
  EXPECT_NO_THROW(handshake.ProcessHelloMessage(json));
}

TEST(HandshakeTest, ProcessHelloRejectsMalformedJson) {
  Handshake handshake("token");
  std::string json = R"({invalid})";
  
  EXPECT_THROW(handshake.ProcessHelloMessage(json), IpcProtocolException);
}

TEST(HandshakeTest, ProcessHelloRejectsInvalidToken) {
  Handshake handshake("correct_token");
  std::string json = R"({"type":"HELLO","sessionToken":"wrong_token"})";
  
  EXPECT_THROW(handshake.ProcessHelloMessage(json), IpcProtocolException);
}

TEST(HandshakeTest, ProcessHelloRejectsWrongMessageType) {
  Handshake handshake("token");
  std::string json = R"({"type":"NAVIGATE","url":"/page"})";
  
  EXPECT_THROW(handshake.ProcessHelloMessage(json), IpcProtocolException);
}

// ============================================================================
// Test: Integration - Full Protocol Flow
// ============================================================================

TEST(HandshakeIntegrationTest, ValidHelloFlow) {
  Handshake handshake("session_abc_123");
  MessageParser parser;
  
  std::string hello_json = R"({
    "type": "HELLO",
    "sessionToken": "session_abc_123"
  })";
  
  auto message = parser.Parse(hello_json);
  ASSERT_EQ(message->GetType(), MessageType::HELLO);
  
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  ASSERT_NE(hello, nullptr);
  
  EXPECT_NO_THROW(handshake.ValidateHello(*hello));
}

TEST(HandshakeIntegrationTest, InvalidHelloFlow) {
  Handshake handshake("session_abc_123");
  MessageParser parser;
  
  std::string hello_json = R"({
    "type": "HELLO",
    "sessionToken": "wrong_token"
  })";
  
  auto message = parser.Parse(hello_json);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  
  EXPECT_THROW(handshake.ValidateHello(*hello), IpcProtocolException);
}

// ============================================================================
// Test: Edge Cases - Special Characters and Long Strings
// ============================================================================

TEST(MessageParserTest, ParseTokenWithSpecialCharacters) {
  MessageParser parser;
  std::string json = R"({"type":"HELLO","sessionToken":"token-with_special.chars@123"})";
  
  auto message = parser.Parse(json);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  EXPECT_EQ(hello->GetSessionToken(), "token-with_special.chars@123");
}

TEST(MessageParserTest, ParseLongSessionToken) {
  MessageParser parser;
  std::string long_token(256, 'a');
  std::string json = R"({"type":"HELLO","sessionToken":")" + long_token + R"("})";
  
  auto message = parser.Parse(json);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  EXPECT_EQ(hello->GetSessionToken(), long_token);
}

TEST(MessageParserTest, ParseUrlWithQueryParameters) {
  MessageParser parser;
  std::string json = R"({"type":"NAVIGATE","url":"/docs/page?param1=value1&param2=value2"})";
  
  auto message = parser.Parse(json);
  auto nav = dynamic_cast<NavigateMessageImpl*>(message.get());
  EXPECT_EQ(nav->GetUrl(), "/docs/page?param1=value1&param2=value2");
}

// ============================================================================
// Test: Error Messages Are Descriptive
// ============================================================================

TEST(MessageParserTest, ErrorMessageForMalformedJson) {
  MessageParser parser;
  std::string json = R"({bad json)";
  
  try {
    parser.Parse(json);
    FAIL() << "Expected IpcProtocolException";
  } catch (const IpcProtocolException& e) {
    // Error message should mention JSON parsing issue
    std::string error = e.what();
    EXPECT_FALSE(error.empty());
  }
}

TEST(HandshakeTest, ErrorMessageForInvalidToken) {
  Handshake handshake("expected");
  HelloMessageImpl hello("actual");
  
  try {
    handshake.ValidateHello(hello);
    FAIL() << "Expected IpcProtocolException";
  } catch (const IpcProtocolException& e) {
    // Error message should mention token mismatch
    std::string error = e.what();
    EXPECT_FALSE(error.empty());
  }
}
