#pragma once

#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include "MessageTypes.h"

// Simple JSON parser stub for Phase 3 (no external libraries yet)
// Just enough to parse HELLO and NAVIGATE messages

namespace cef_ui {
namespace ipc {

// Forward declaration
class IpcProtocolException;

// Simple JSON value types for parsing
class JsonValue {
 public:
  enum Type { NIL, BOOLEAN, NUMBER, STRING, OBJECT, ARRAY };
  
  Type type;
  std::string string_value;
  bool bool_value;
  double number_value;
  std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> object_value;
  
  JsonValue(Type t = NIL) : type(t), bool_value(false), number_value(0) {}
};

// Minimal JSON parser for testing
class SimpleJsonParser {
 public:
  static std::unique_ptr<JsonValue> Parse(const std::string& json_str);
  static std::string GetStringValue(const JsonValue* obj, const std::string& key);
  static bool TryGetStringValue(const JsonValue* obj, const std::string& key, std::string& out);
};

}  // namespace ipc
}  // namespace cef_ui
