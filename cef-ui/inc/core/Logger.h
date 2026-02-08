#pragma once

#include <string>

enum class LogLevel {
    Info,
    Warn,
    Error
};

class Logger {
public:
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

    // Optional: overloads with context
    static void info(const std::string& context, const std::string& message);
    static void warn(const std::string& context, const std::string& message);
    static void error(const std::string& context, const std::string& message);

private:
    static void log(LogLevel level, const std::string& message);
    static void log(LogLevel level, const std::string& context, const std::string& message);
};
