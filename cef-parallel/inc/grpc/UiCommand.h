#pragma once

#include <string>
#include <variant>

namespace cef_ui {
namespace grpc_server {

/// Command types that can be sent from gRPC to UI thread
enum class CommandType {
    OPEN_PAGE,
    SHUTDOWN
};

/// Command to open a page in the browser
struct OpenPageCommand {
    std::string command_id;
    std::string url;
    
    OpenPageCommand(const std::string& id, const std::string& page_url)
        : command_id(id), url(page_url) {}
};

/// Command to shutdown the application
struct ShutdownCommand {
    ShutdownCommand() = default;
};

/// Value object representing a UI command
/// Created on gRPC threads, marshaled to CEF UI thread
class UiCommand {
public:
    using CommandData = std::variant<OpenPageCommand, ShutdownCommand>;
    
    explicit UiCommand(OpenPageCommand cmd)
        : type_(CommandType::OPEN_PAGE), data_(std::move(cmd)) {}
    
    explicit UiCommand(ShutdownCommand cmd)
        : type_(CommandType::SHUTDOWN), data_(std::move(cmd)) {}
    
    CommandType GetType() const { return type_; }
    
    const CommandData& GetData() const { return data_; }
    
    // Helper accessors
    const OpenPageCommand* AsOpenPage() const {
        return std::get_if<OpenPageCommand>(&data_);
    }
    
    const ShutdownCommand* AsShutdown() const {
        return std::get_if<ShutdownCommand>(&data_);
    }

private:
    CommandType type_;
    CommandData data_;
};

}  // namespace grpc_server
}  // namespace cef_ui
