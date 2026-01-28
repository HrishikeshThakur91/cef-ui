#include "pch.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "grpc/GrpcServer.h"
#include "grpc/cef_service.grpc.pb.h"

using namespace cef_ui::grpc_server;
using namespace cefcontrol;
using namespace grpc;

// Test Fixture for GrpcServer
class GrpcServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        session_token_ = std::string("test_token_") + std::to_string(std::rand());
        server_ = std::make_unique<GrpcServer>(session_token_);
        
        // Use a random port range to reduce collisions in concurrent test runs
        // (This is rudimentary but sufficient for now)
        port_ = 50050 + (std::rand() % 100);
        
        ASSERT_TRUE(server_->Start(port_));
        
        std::string address = "localhost:" + std::to_string(port_);
        channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        stub_ = CefControlService::NewStub(channel_);
    }

    void TearDown() override {
        if (server_) {
            server_->Stop();
        }
    }

    // Helper to perform handshake
    bool PerformHandshake(const std::string& token, const std::string& version = "1.0") {
        ClientContext context;
        HandshakeRequest request;
        HandshakeResponse response;
        request.set_session_token(token);
        request.set_client_version(version);
        
        Status status = stub_->Handshake(&context, request, &response);
        return status.ok() && response.success();
    }

    std::string session_token_;
    uint16_t port_;
    std::unique_ptr<GrpcServer> server_;
    std::shared_ptr<Channel> channel_;
    std::unique_ptr<CefControlService::Stub> stub_;
};

// ============================================================================
// Test: Handshake Logic
// ============================================================================

TEST_F(GrpcServerTest, HandshakeSuccess) {
    ClientContext context;
    HandshakeRequest request;
    HandshakeResponse response;
    
    request.set_session_token(session_token_);
    request.set_client_version("1.0.0");
    
    Status status = stub_->Handshake(&context, request, &response);
    
    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(response.success());
    EXPECT_EQ(response.message(), "Handshake accepted");
    EXPECT_FALSE(response.server_version().empty());
}

TEST_F(GrpcServerTest, HandshakeFailsWithInvalidToken) {
    ClientContext context;
    HandshakeRequest request;
    HandshakeResponse response;
    
    request.set_session_token("wrong_token");
    request.set_client_version("1.0.0");
    
    Status status = stub_->Handshake(&context, request, &response);
    
    EXPECT_TRUE(status.ok()); // RPC succeeds
    EXPECT_FALSE(response.success()); // Logic fails
    EXPECT_NE(response.message().find("Invalid session token"), std::string::npos);
}

TEST_F(GrpcServerTest, HandshakeFailsWithMissingVersion) {
    ClientContext context;
    HandshakeRequest request;
    HandshakeResponse response;
    
    request.set_session_token(session_token_);
    // Missing client version
    
    Status status = stub_->Handshake(&context, request, &response);
    
    EXPECT_TRUE(status.ok());
    EXPECT_FALSE(response.success());
    EXPECT_NE(response.message().find("Missing client version"), std::string::npos);
}

// ============================================================================
// Test: OpenPage Logic (Phase 6.2)
// ============================================================================

TEST_F(GrpcServerTest, OpenPageRejectedBeforeHandshake) {
    ClientContext context;
    OpenPageRequest request;
    OpenPageResponse response;
    
    request.set_command_id("cmd1");
    request.set_page_url("http://example.com");
    
    Status status = stub_->OpenPage(&context, request, &response);
    
    EXPECT_TRUE(status.ok());
    EXPECT_FALSE(response.accepted());
    EXPECT_EQ(response.message(), "Handshake required");
}

TEST_F(GrpcServerTest, OpenPageAcceptedAfterHandshake) {
    ASSERT_TRUE(PerformHandshake(session_token_));
    
    ClientContext context;
    OpenPageRequest request;
    OpenPageResponse response;
    
    request.set_command_id("cmd2");
    request.set_page_url("http://google.com");
    
    Status status = stub_->OpenPage(&context, request, &response);
    
    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(response.accepted());
    // Phase 6.3: Command should be accepted and queued for execution
    EXPECT_NE(response.message().find("accepted and queued for execution"), std::string::npos);
    EXPECT_EQ(response.command_id(), "cmd2");
}

TEST_F(GrpcServerTest, OpenPageRejectedMissingFields) {
    ASSERT_TRUE(PerformHandshake(session_token_));
    
    ClientContext context;
    OpenPageRequest request;
    OpenPageResponse response;
    
    // Missing command_id
    request.set_page_url("http://google.com");
    
    Status status = stub_->OpenPage(&context, request, &response);
    
    EXPECT_TRUE(status.ok());
    EXPECT_FALSE(response.accepted());
    EXPECT_EQ(response.message(), "Missing command_id");
}

// ============================================================================
// Test: PageStatus Logic (Phase 6.2 Placeholder)
// ============================================================================

TEST_F(GrpcServerTest, PageStatusMaintainsPlaceholder) {
    ASSERT_TRUE(PerformHandshake(session_token_));
    
    ClientContext context;
    PageStatusRequest request;
    PageStatusResponse response;
    
    request.set_command_id("cmd3");
    
    Status status = stub_->PageStatus(&context, request, &response);
    
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.status(), "UNKNOWN");
    EXPECT_NE(response.message().find("not implemented in Phase 6.2"), std::string::npos);
    EXPECT_EQ(response.command_id(), "cmd3");
}

// ============================================================================
// Test: Shutdown Logic (Phase 6.2 Stub)
// ============================================================================

TEST_F(GrpcServerTest, ShutdownAcknowledgedAndQueued) {
    // Phase 6.3: Shutdown creates command and posts to UI thread
    
    ClientContext context;
    ShutdownRequest request;
    ShutdownResponse response;
    
    Status status = stub_->Shutdown(&context, request, &response);
    
    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(response.acknowledged());
    EXPECT_NE(response.message().find("acknowledged and queued for execution"), std::string::npos);
}
