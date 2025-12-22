#include "../../inc/ipc/JsonParser.h"
#include <sstream>
#include <cctype>
#include <cstdlib>

namespace cef_ui {
namespace ipc {

class JsonTokenizer {
 public:
  enum TokenType {
    TOK_LBRACE, TOK_RBRACE, TOK_LBRACKET, TOK_RBRACKET,
    TOK_COLON, TOK_COMMA,
    TOK_STRING, TOK_NUMBER, TOK_TRUE, TOK_FALSE, TOK_NULL,
    TOK_EOF, TOK_ERROR
  };

  struct Token {
    TokenType type;
    std::string value;
  };

  JsonTokenizer(const std::string& json) : json_(json), pos_(0) {}

  Token GetNextToken() {
    SkipWhitespace();

    if (pos_ >= json_.length()) {
      return { TOK_EOF, "" };
    }

    char ch = json_[pos_];

    if (ch == '{') {
      pos_++;
      return { TOK_LBRACE, "{" };
    }
    if (ch == '}') {
      pos_++;
      return { TOK_RBRACE, "}" };
    }
    if (ch == '[') {
      pos_++;
      return { TOK_LBRACKET, "[" };
    }
    if (ch == ']') {
      pos_++;
      return { TOK_RBRACKET, "]" };
    }
    if (ch == ':') {
      pos_++;
      return { TOK_COLON, ":" };
    }
    if (ch == ',') {
      pos_++;
      return { TOK_COMMA, "," };
    }
    if (ch == '"') {
      return ParseString();
    }
    if (ch == 't' && json_.substr(pos_, 4) == "true") {
      pos_ += 4;
      return { TOK_TRUE, "true" };
    }
    if (ch == 'f' && json_.substr(pos_, 5) == "false") {
      pos_ += 5;
      return { TOK_FALSE, "false" };
    }
    if (ch == 'n' && json_.substr(pos_, 4) == "null") {
      pos_ += 4;
      return { TOK_NULL, "null" };
    }
    if (ch == '-' || (ch >= '0' && ch <= '9')) {
      return ParseNumber();
    }

    return { TOK_ERROR, "Unexpected character" };
  }

 private:
  std::string json_;
  size_t pos_;

  void SkipWhitespace() {
    while (pos_ < json_.length() && std::isspace(json_[pos_])) {
      pos_++;
    }
  }

  Token ParseString() {
    pos_++;  // Skip opening quote
    std::string result;

    while (pos_ < json_.length() && json_[pos_] != '"') {
      if (json_[pos_] == '\\' && pos_ + 1 < json_.length()) {
        pos_++;
        char next = json_[pos_];
        if (next == '"' || next == '\\' || next == '/') {
          result += next;
        } else if (next == 'b') {
          result += '\b';
        } else if (next == 'f') {
          result += '\f';
        } else if (next == 'n') {
          result += '\n';
        } else if (next == 'r') {
          result += '\r';
        } else if (next == 't') {
          result += '\t';
        } else {
          result += next;
        }
      } else {
        result += json_[pos_];
      }
      pos_++;
    }

    if (pos_ < json_.length()) {
      pos_++;  // Skip closing quote
    }

    return { TOK_STRING, result };
  }

  Token ParseNumber() {
    size_t start = pos_;
    
    if (json_[pos_] == '-') {
      pos_++;
    }

    while (pos_ < json_.length() && std::isdigit(json_[pos_])) {
      pos_++;
    }

    if (pos_ < json_.length() && json_[pos_] == '.') {
      pos_++;
      while (pos_ < json_.length() && std::isdigit(json_[pos_])) {
        pos_++;
      }
    }

    if (pos_ < json_.length() && (json_[pos_] == 'e' || json_[pos_] == 'E')) {
      pos_++;
      if (pos_ < json_.length() && (json_[pos_] == '+' || json_[pos_] == '-')) {
        pos_++;
      }
      while (pos_ < json_.length() && std::isdigit(json_[pos_])) {
        pos_++;
      }
    }

    std::string num = json_.substr(start, pos_ - start);
    return { TOK_NUMBER, num };
  }
};

class JsonParser {
 public:
  explicit JsonParser(const std::string& json)
      : tokenizer_(json), current_token_({ JsonTokenizer::TOK_EOF, "" }) {
    Advance();
  }

  std::unique_ptr<JsonValue> Parse() {
    return ParseValue();
  }

 private:
  JsonTokenizer tokenizer_;
  JsonTokenizer::Token current_token_;

  void Advance() {
    current_token_ = tokenizer_.GetNextToken();
  }

  std::unique_ptr<JsonValue> ParseValue() {
    if (current_token_.type == JsonTokenizer::TOK_LBRACE) {
      return ParseObject();
    } else if (current_token_.type == JsonTokenizer::TOK_LBRACKET) {
      return ParseArray();
    } else if (current_token_.type == JsonTokenizer::TOK_STRING) {
      auto result = std::make_unique<JsonValue>(JsonValue::STRING);
      result->string_value = current_token_.value;
      Advance();
      return result;
    } else if (current_token_.type == JsonTokenizer::TOK_NUMBER) {
      auto result = std::make_unique<JsonValue>(JsonValue::NUMBER);
      result->number_value = std::stod(current_token_.value);
      Advance();
      return result;
    } else if (current_token_.type == JsonTokenizer::TOK_TRUE) {
      auto result = std::make_unique<JsonValue>(JsonValue::BOOLEAN);
      result->bool_value = true;
      Advance();
      return result;
    } else if (current_token_.type == JsonTokenizer::TOK_FALSE) {
      auto result = std::make_unique<JsonValue>(JsonValue::BOOLEAN);
      result->bool_value = false;
      Advance();
      return result;
    } else if (current_token_.type == JsonTokenizer::TOK_NULL) {
      Advance();
      return std::make_unique<JsonValue>(JsonValue::NIL);
    }
    return nullptr;
  }

  std::unique_ptr<JsonValue> ParseObject() {
    auto obj = std::make_unique<JsonValue>(JsonValue::OBJECT);
    
    Advance();  // Skip '{'

    while (current_token_.type != JsonTokenizer::TOK_RBRACE &&
           current_token_.type != JsonTokenizer::TOK_EOF) {
      if (current_token_.type != JsonTokenizer::TOK_STRING) {
        return nullptr;
      }

      std::string key = current_token_.value;
      Advance();

      if (current_token_.type != JsonTokenizer::TOK_COLON) {
        return nullptr;
      }

      Advance();

      auto value = ParseValue();
      if (!value) {
        return nullptr;
      }

      obj->object_value.push_back({ key, std::move(value) });

      if (current_token_.type == JsonTokenizer::TOK_COMMA) {
        Advance();
      } else if (current_token_.type != JsonTokenizer::TOK_RBRACE) {
        return nullptr;
      }
    }

    Advance();  // Skip '}'
    return obj;
  }

  std::unique_ptr<JsonValue> ParseArray() {
    auto arr = std::make_unique<JsonValue>(JsonValue::ARRAY);
    
    Advance();  // Skip '['

    while (current_token_.type != JsonTokenizer::TOK_RBRACKET &&
           current_token_.type != JsonTokenizer::TOK_EOF) {
      auto value = ParseValue();
      if (!value) {
        return nullptr;
      }

      // Store in object_value as dummy (arrays not fully implemented)
      arr->object_value.push_back({ "", std::move(value) });

      if (current_token_.type == JsonTokenizer::TOK_COMMA) {
        Advance();
      } else if (current_token_.type != JsonTokenizer::TOK_RBRACKET) {
        return nullptr;
      }
    }

    Advance();  // Skip ']'
    return arr;
  }
};

// SimpleJsonParser implementation
std::unique_ptr<JsonValue> SimpleJsonParser::Parse(const std::string& json_str) {
  if (json_str.empty()) {
    return nullptr;
  }

  JsonParser parser(json_str);
  return parser.Parse();
}

std::string SimpleJsonParser::GetStringValue(const JsonValue* obj, const std::string& key) {
  std::string result;
  if (TryGetStringValue(obj, key, result)) {
    return result;
  }
  throw std::runtime_error("Key not found or value is not a string: " + key);
}

bool SimpleJsonParser::TryGetStringValue(const JsonValue* obj, const std::string& key, std::string& out) {
  if (!obj || obj->type != JsonValue::OBJECT) {
    return false;
  }

  for (const auto& pair : obj->object_value) {
    if (pair.first == key && pair.second && pair.second->type == JsonValue::STRING) {
      out = pair.second->string_value;
      return true;
    }
  }

  return false;
}

}  // namespace ipc
}  // namespace cef_ui
