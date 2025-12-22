#include "pch.h"
#include <memory>
#include <string>
#include <gtest/gtest.h>

#include "ipc/WebSocketTransport.h"
#include "ipc/IWebSocketConnection.h"
#include "ipc/ITlsContextProvider.h"
#include "../support/mock_websocket_connection.h"
#include "../support/mock_tls_context_provider.h"

using namespace cef_ui::ipc;

// ============================================================================
// Test: WebSocketTransport - Constructor and Initialization
// ============================================================================

TEST(WebSocketTransportTest, ConstructorInitializesWithDependencies) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();

  // Instantiate REAL WebSocketTransport with injected mocks
  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  // Should be created but not connected yet
  EXPECT_FALSE(transport.IsConnected());
}

// ============================================================================
// Test: WebSocketTransport - Connect Method
// ============================================================================

TEST(WebSocketTransportTest, ConnectCallsWebSocketConnectionWithCorrectParameters) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "api.example.com", 443);
  
  // Call REAL Connect method
  transport.Connect();

  // Verify mock was called with correct host/port
  EXPECT_EQ(mock_conn->GetConnectCallCount(), 1);
  auto hosts = mock_conn->GetConnectHosts();
  EXPECT_EQ(hosts.size(), 1);
  EXPECT_EQ(hosts[0], "api.example.com");
  
  auto ports = mock_conn->GetConnectPorts();
  EXPECT_EQ(ports.size(), 1);
  EXPECT_EQ(ports[0], 443);
}

TEST(WebSocketTransportTest, ConnectThrowsWhenWebSocketConnectionFails) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectFailure("Connection refused");
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  // Call REAL Connect - should throw because mock fails
  EXPECT_THROW(transport.Connect(), std::runtime_error);
}

TEST(WebSocketTransportTest, ConnectThrowsWhenTlsProviderUnavailable) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_tls->SetAvailable(false);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  // Call REAL Connect - should throw because TLS provider unavailable
  EXPECT_THROW(transport.Connect(), std::runtime_error);
}

TEST(WebSocketTransportTest, ConnectRequestsTlsContextFromProvider) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "secure.service.local", 443);
  
  // Call REAL Connect method
  transport.Connect();

  // Verify TLS provider was called with correct hostname
  auto created_for = mock_tls->GetCreatedContextsFor();
  EXPECT_EQ(created_for.size(), 1);
  EXPECT_EQ(created_for[0], "secure.service.local");
}

TEST(WebSocketTransportTest, ConnectSetIsConnectedState) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  EXPECT_FALSE(transport.IsConnected());
  
  // Call REAL Connect
  transport.Connect();
  
  // After successful connect, should report as connected
  EXPECT_TRUE(transport.IsConnected());
}

// ============================================================================
// Test: WebSocketTransport - Send Method (IMessageChannel interface)
// ============================================================================

TEST(WebSocketTransportTest, SendForwardsMessageToWebSocketConnection) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  transport.Connect();

  // Call REAL Send method with message
  std::string message = R"({"type":"HELLO","sessionToken":"abc123"})";
  transport.Send(message);

  // Verify mock connection received the exact message
  auto sent = mock_conn->GetSentMessages();
  EXPECT_EQ(sent.size(), 1);
  EXPECT_EQ(sent[0], message);
}

TEST(WebSocketTransportTest, SendThrowsWhenNotConnected) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  // Call REAL Send without connecting first
  EXPECT_THROW(transport.Send("test message"), std::runtime_error);
}

TEST(WebSocketTransportTest, SendThrowsWhenConnectionFails) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  transport.Connect();
  
  // Simulate connection failure on send
  mock_conn->SetSendFailure("Send failed: connection lost");
  
  // Call REAL Send - should throw
  EXPECT_THROW(transport.Send("test"), std::runtime_error);
}

// ============================================================================
// Test: WebSocketTransport - Receive Method (IMessageChannel interface)
// ============================================================================

TEST(WebSocketTransportTest, ReceiveReturnsDataFromWebSocketConnection) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  transport.Connect();

  // Set up response in mock
  std::string response = R"({"type":"NAVIGATE","url":"/page"})";
  mock_conn->SetNextResponse(response);

  // Call REAL Receive method
  std::string received = transport.Receive();

  // Verify received the exact message from mock
  EXPECT_EQ(received, response);
}

TEST(WebSocketTransportTest, ReceiveThrowsWhenNotConnected) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  // Call REAL Receive without connecting first
  EXPECT_THROW(transport.Receive(), std::runtime_error);
}

TEST(WebSocketTransportTest, ReceiveThrowsWhenConnectionFails) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  transport.Connect();
  
  // Simulate receive timeout/failure
  mock_conn->SetReceiveFailure("Receive timeout");
  
  // Call REAL Receive - should throw
  EXPECT_THROW(transport.Receive(), std::runtime_error);
}

// ============================================================================
// Test: WebSocketTransport - IsConnected Method
// ============================================================================

TEST(WebSocketTransportTest, IsConnectedReturnsFalseInitially) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  // Call REAL IsConnected - should return false before connect
  EXPECT_FALSE(transport.IsConnected());
}

TEST(WebSocketTransportTest, IsConnectedReturnsTrueAfterConnect) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  transport.Connect();
  
  // Call REAL IsConnected - should return true
  EXPECT_TRUE(transport.IsConnected());
}

// ============================================================================
// Test: WebSocketTransport - Close Method
// ============================================================================

TEST(WebSocketTransportTest, CloseDisconnectsConnection) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  transport.Connect();
  
  EXPECT_TRUE(transport.IsConnected());
  
  // Call REAL Close method
  transport.Close();
  
  // Verify close was called on mock connection
  EXPECT_EQ(mock_conn->GetCloseCallCount(), 1);
  EXPECT_FALSE(transport.IsConnected());
}

TEST(WebSocketTransportTest, CloseThrowsOnConnectionCloseFailure) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  transport.Connect();
  
  // Close without setting up failure should work fine
  EXPECT_NO_THROW(transport.Close());
}

// ============================================================================
// Test: WebSocketTransport - IMessageChannel Interface Contract
// ============================================================================

TEST(WebSocketTransportTest, ImplementsIMessageChannelInterface) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();

  // WebSocketTransport should be assignable to IMessageChannel pointer
  std::unique_ptr<IMessageChannel> channel = 
      std::make_unique<WebSocketTransport>(mock_conn, mock_tls, "localhost", 443);
  
  EXPECT_NE(channel, nullptr);
}

// ============================================================================
// Test: WebSocketTransport - Multiple Connect/Disconnect Cycles
// ============================================================================

TEST(WebSocketTransportTest, CanConnectAfterDisconnect) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  WebSocketTransport transport(mock_conn, mock_tls, "localhost", 443);
  
  // First cycle
  transport.Connect();
  EXPECT_TRUE(transport.IsConnected());
  transport.Close();
  EXPECT_FALSE(transport.IsConnected());
  
  // Second cycle
  transport.Connect();
  EXPECT_TRUE(transport.IsConnected());
}

// ============================================================================
// Test: WebSocketTransport - Host and Port Configuration
// ============================================================================

TEST(WebSocketTransportTest, UsesConfiguredHostAndPortInConnect) {
  auto mock_conn = std::make_shared<MockWebSocketConnection>();
  auto mock_tls = std::make_shared<MockTlsContextProvider>();
  mock_conn->SetConnectSuccess();
  mock_tls->SetAvailable(true);

  // Create with specific host and port
  const std::string host = "ws.api.production.com";
  const uint16_t port = 8443;
  
  WebSocketTransport transport(mock_conn, mock_tls, host, port);
  transport.Connect();

  // Verify correct host and port were passed to connection
  auto hosts = mock_conn->GetConnectHosts();
  auto ports = mock_conn->GetConnectPorts();
  
  EXPECT_EQ(hosts[0], host);
  EXPECT_EQ(ports[0], port);
}
