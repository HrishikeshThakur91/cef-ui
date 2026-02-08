#include "core/Logger.h"
#include <iostream>
#include <mutex>

namespace {
    std::mutex logMutex;
}

void Logger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::Warn, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logger::info(const std::string& context, const std::string& message) {
    log(LogLevel::Info, context, message);
}

void Logger::warn(const std::string& context, const std::string& message) {
    log(LogLevel::Warn, context, message);
}

void Logger::error(const std::string& context, const std::string& message) {
    log(LogLevel::Error, context, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    out << "[LOG] " << message << std::endl;
}

void Logger::log(LogLevel level, const std::string& context, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;
    out << "[LOG][" << context << "] " << message << std::endl;
}
