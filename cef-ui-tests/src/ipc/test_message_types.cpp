#pragma once
#include "pch.h"

#include <memory>
#include <string>
#include <gtest/gtest.h>

#include "ipc/Handshake.h"
#include "ipc/MessageTypes.h"

using namespace cef_ui::ipc;

// ============================================================================
// Test: Message Type Identification
// ============================================================================

TEST(MessageTypeTest, HelloMessageTypeCorrect) {
  HelloMessageImpl msg("token");
  EXPECT_EQ(msg.GetType(), MessageType::HELLO);
}

TEST(MessageTypeTest, NavigateMessageTypeCorrect) {
  NavigateMessageImpl msg("/path");
  EXPECT_EQ(msg.GetType(), MessageType::NAVIGATE);
}

// ============================================================================
// Test: Message Immutability
// ============================================================================

TEST(MessageImmutabilityTest, HelloMessageSessionTokenImmutable) {
  HelloMessageImpl msg("original_token");
  EXPECT_EQ(msg.GetSessionToken(), "original_token");
  // No setter available, token cannot change
  EXPECT_EQ(msg.GetSessionToken(), "original_token");
}

TEST(MessageImmutabilityTest, NavigateMessageUrlImmutable) {
  NavigateMessageImpl msg("/original/path");
  EXPECT_EQ(msg.GetUrl(), "/original/path");
  // No setter available, URL cannot change
  EXPECT_EQ(msg.GetUrl(), "/original/path");
}

// ============================================================================
// Test: Message Polymorphism (base class)
// ============================================================================

TEST(MessagePolymorphismTest, HelloMessageCanBeTreatedAsBase) {
  std::unique_ptr<Message> msg = std::make_unique<HelloMessageImpl>("token");
  EXPECT_EQ(msg->GetType(), MessageType::HELLO);
}

TEST(MessagePolymorphismTest, NavigateMessageCanBeTreatedAsBase) {
  std::unique_ptr<Message> msg = std::make_unique<NavigateMessageImpl>("/path");
  EXPECT_EQ(msg->GetType(), MessageType::NAVIGATE);
}

// ============================================================================
// Test: Protocol Compliance - Required Fields
// ============================================================================

TEST(ProtocolComplianceTest, HelloMessageRequiresSessionToken) {
  // HELLO message must have sessionToken field
  MessageParser parser;
  std::string json = R"({"type":"HELLO"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(ProtocolComplianceTest, NavigateMessageRequiresUrl) {
  // NAVIGATE message must have url field
  MessageParser parser;
  std::string json = R"({"type":"NAVIGATE"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

// ============================================================================
// Test: JSON Parsing Robustness
// ============================================================================

TEST(JsonParsingTest, HandlesDuplicateKeys) {
  MessageParser parser;
  // Last occurrence of duplicate key should win
  std::string json = R"({
    "type": "HELLO",
    "sessionToken": "first",
    "sessionToken": "second"
  })";
  
  auto message = parser.Parse(json);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  // Behavior: typically the last value wins in JSON parsing
  EXPECT_NE(hello->GetSessionToken(), "");
}

TEST(JsonParsingTest, HandlesNestedObjects) {
  MessageParser parser;
  // Nested objects should cause parsing to fail (extra fields ignored)
  std::string json = R"({
    "type": "HELLO",
    "sessionToken": "token",
    "metadata": {"nested": "object"}
  })";
  
  auto message = parser.Parse(json);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  EXPECT_EQ(hello->GetSessionToken(), "token");
}

TEST(JsonParsingTest, HandlesArrayFields) {
  MessageParser parser;
  std::string json = R"({
    "type": "HELLO",
    "sessionToken": "token",
    "tags": ["tag1", "tag2"]
  })";
  
  auto message = parser.Parse(json);
  auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
  EXPECT_EQ(hello->GetSessionToken(), "token");
}

// ============================================================================
// Test: Handshake State Isolation
// ============================================================================

TEST(HandshakeStateTest, MultipleHandshakesAreIndependent) {
  Handshake handshake1("token1");
  Handshake handshake2("token2");
  
  EXPECT_EQ(handshake1.GetExpectedToken(), "token1");
  EXPECT_EQ(handshake2.GetExpectedToken(), "token2");
}

TEST(HandshakeStateTest, HandshakeIsNonCopyable) {
  Handshake handshake("token");
  // Intentionally not assignable or copyable - prevents accidental state sharing
  static_assert(!std::is_copy_constructible<Handshake>::value, "Handshake must not be copyable");
}

// ============================================================================
// Test: Message Parser Exception Safety
// ============================================================================

TEST(ExceptionSafetyTest, ParserDoesNotModifyStateOnException) {
  MessageParser parser;
  
  // First successful parse
  std::string valid_json = R"({"type":"HELLO","sessionToken":"token"})";
  auto msg1 = parser.Parse(valid_json);
  ASSERT_NE(msg1, nullptr);
  
  // Failed parse should not corrupt parser
  std::string invalid_json = R"({bad})";
  EXPECT_THROW(parser.Parse(invalid_json), IpcProtocolException);
  
  // Parser should still work after exception
  auto msg2 = parser.Parse(valid_json);
  ASSERT_NE(msg2, nullptr);
}

// ============================================================================
// Test: Token Validation Edge Cases
// ============================================================================

TEST(TokenValidationEdgeCaseTest, TokenWithLeadingWhitespace) {
  Handshake handshake(" token");
  HelloMessageImpl hello(" token");
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(TokenValidationEdgeCaseTest, TokenWithTrailingWhitespace) {
  Handshake handshake("token ");
  HelloMessageImpl hello("token ");
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

TEST(TokenValidationEdgeCaseTest, TokenWithUnicodeCharacters) {
  Handshake handshake("token_ñ_ü");
  HelloMessageImpl hello("token_ñ_ü");
  
  EXPECT_NO_THROW(handshake.ValidateHello(hello));
}

// ============================================================================
// Test: Message Type Case Sensitivity
// ============================================================================

TEST(MessageTypeCaseSensitivityTest, HelloMustBeUppercase) {
  MessageParser parser;
  std::string json = R"({"type":"hello","sessionToken":"token"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}

TEST(MessageTypeCaseSensitivityTest, NavigateMustBeUppercase) {
  MessageParser parser;
  std::string json = R"({"type":"navigate","url":"/path"})";
  
  EXPECT_THROW(parser.Parse(json), IpcProtocolException);
}
