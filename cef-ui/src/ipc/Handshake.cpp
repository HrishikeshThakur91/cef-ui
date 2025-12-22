#include "../../inc/ipc/Handshake.h"
#include "../../inc/ipc/JsonParser.h"
#include <stdexcept>

namespace cef_ui {
    namespace ipc {

        // Trims leading and trailing ASCII whitespace.
    // Intentionally simple and deterministic.
        std::string TrimWhitespace(const std::string& input) {
            const auto begin = input.find_first_not_of(" \t\n\r");
            if (begin == std::string::npos) {
                return "";
            }

            const auto end = input.find_last_not_of(" \t\n\r");
            return input.substr(begin, end - begin + 1);
        }

        // MessageParser implementation
        std::unique_ptr<Message> MessageParser::Parse(const std::string& json_string) {
            if (json_string.empty()) {
                throw IpcProtocolException("JSON string is empty");
            }

            // Use simple JSON parser
            auto json = SimpleJsonParser::Parse(json_string);

            if (!json || json->type != JsonValue::OBJECT) {
                throw IpcProtocolException("JSON must be an object");
            }

            // Get type field
            std::string type_str;
            if (!SimpleJsonParser::TryGetStringValue(json.get(), "type", type_str)) {
                throw IpcProtocolException("Missing 'type' field in message");
            }

            MessageType msg_type = GetMessageType(type_str);

            if (msg_type == MessageType::HELLO) {
                std::string token;
                if (!SimpleJsonParser::TryGetStringValue(json.get(), "sessionToken", token)) {
                    throw IpcProtocolException("Missing 'sessionToken' field in HELLO message");
                }
                return std::make_unique<HelloMessageImpl>(token);
            }
            else if (msg_type == MessageType::NAVIGATE) {
                std::string url;
                if (!SimpleJsonParser::TryGetStringValue(json.get(), "url", url)) {
                    throw IpcProtocolException("Missing 'url' field in NAVIGATE message");
                }
                return std::make_unique<NavigateMessageImpl>(url);
            }

            throw IpcProtocolException("Unknown message type");
        }

        MessageType MessageParser::GetMessageType(const std::string& type_string) {
            if (type_string == "HELLO") {
                return MessageType::HELLO;
            }
            else if (type_string == "NAVIGATE") {
                return MessageType::NAVIGATE;
            }
            return MessageType::UNKNOWN;
        }

        // Handshake implementation
        Handshake::Handshake(const std::string& expected_token)
            : expected_token_(expected_token) {
        }

        void Handshake::ValidateHello(const HelloMessageImpl& message) {
            if (expected_token_.empty()) {
                throw IpcProtocolException("Expected token cannot be empty");
            }

            // Trim the RECEIVED token only (do NOT modify expected token)
            const std::string received = TrimWhitespace(message.GetSessionToken());

            // Compare trimmed received with expected (unmodified)
            if (received != expected_token_) {
                throw IpcProtocolException("Session token mismatch");
            }
        }

        const std::string& Handshake::GetExpectedToken() const {
            return expected_token_;
        }

        void Handshake::ProcessHelloMessage(const std::string& json_string) {
            auto message = parser_.Parse(json_string);

            if (message->GetType() != MessageType::HELLO) {
                throw IpcProtocolException("Expected HELLO message");
            }

            auto hello = dynamic_cast<HelloMessageImpl*>(message.get());
            if (!hello) {
                throw IpcProtocolException("Failed to parse HELLO message");
            }

            ValidateHello(*hello);
        }

    }  // namespace ipc
}  // namespace cef_ui
