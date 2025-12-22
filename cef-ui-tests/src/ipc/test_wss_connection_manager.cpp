#include "pch.h"
#include <memory>
#include <string>
#include <vector>
#include "ipc/WssConnectionManager.h"
#include "ipc/IMessageChannel.h"
#include "../support/mock_websocket_connection.h"
#include "../support/mock_connection_listener.h"

using namespace cef_ui::ipc;

// ============================================================================
// Test: WssConnectionManager - Constructor and Initialization
// ============================================================================

TEST(WssConnectionManagerTest, ConstructorInitializesWithChannel) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();

  // Instantiate REAL WssConnectionManager with injected IMessageChannel mock
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  // Should not be ready until connected
  EXPECT_FALSE(manager.IsReady());
}

// ============================================================================
// Test: WssConnectionManager - Connect on First Attempt Succeeds
// ============================================================================

TEST(WssConnectionManagerTest, ConnectEmitsConnectingEvent) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  mock_channel->SetConnectSuccess();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  auto listener = std::make_shared<MockConnectionListener>();
  manager.AddListener(listener);

  // Call REAL Connect
  manager.Connect();

  // Verify OnConnecting was emitted
  EXPECT_EQ(listener->GetOnConnectingCallCount(), 1);
}

TEST(WssConnectionManagerTest, ConnectEmitsConnectedEvent) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  mock_channel->SetConnectSuccess();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  auto listener = std::make_shared<MockConnectionListener>();
  manager.AddListener(listener);

  // Call REAL Connect
  manager.Connect();

  // Verify OnConnected was emitted
  EXPECT_EQ(listener->GetOnConnectedCallCount(), 1);
  auto events = listener->GetEventSequence();
  EXPECT_TRUE(events.size() >= 2);
}

TEST(WssConnectionManagerTest, MaxRetriesIs5) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));

  // Call REAL GetRetryStats
  auto stats = manager.GetRetryStats();
  
  EXPECT_EQ(stats.max_retries, 5);
}

TEST(WssConnectionManagerTest, ConnectionTimeoutIs30Seconds) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));

  // Verify timeout is hardcoded to 30 seconds (30000 ms)
  EXPECT_EQ(manager.GetConnectionTimeoutMs(), 30000);
}

// ============================================================================
// Test: WssConnectionManager - Event Ordering
// ============================================================================

TEST(WssConnectionManagerTest, EventOrderingOnSuccessfulConnect) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  mock_channel->SetConnectSuccess();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  auto listener = std::make_shared<MockConnectionListener>();
  manager.AddListener(listener);

  // Call REAL Connect
  manager.Connect();

  // Verify event order: OnConnecting, then OnConnected
  auto events = listener->GetEventSequence();
  EXPECT_GE(events.size(), 2);
  EXPECT_EQ(events[0], "OnConnecting");
  EXPECT_EQ(events[1], "OnConnected");
}

// ============================================================================
// Test: WssConnectionManager - Multiple Listeners
// ============================================================================

TEST(WssConnectionManagerTest, MultipleListenersReceiveConnectingEvent) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  mock_channel->SetConnectSuccess();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  auto listener1 = std::make_shared<MockConnectionListener>();
  auto listener2 = std::make_shared<MockConnectionListener>();
  auto listener3 = std::make_shared<MockConnectionListener>();
  
  manager.AddListener(listener1);
  manager.AddListener(listener2);
  manager.AddListener(listener3);

  // Call REAL Connect
  manager.Connect();

  // Verify all listeners received OnConnecting
  EXPECT_EQ(listener1->GetOnConnectingCallCount(), 1);
  EXPECT_EQ(listener2->GetOnConnectingCallCount(), 1);
  EXPECT_EQ(listener3->GetOnConnectingCallCount(), 1);
}

// ============================================================================
// Test: WssConnectionManager - Send/Receive
// ============================================================================

TEST(WssConnectionManagerTest, SendThrowsWhenNotConnected) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));

  // Call REAL SendMessage without connecting
  EXPECT_THROW(manager.SendMessage("test"), std::runtime_error);
}

TEST(WssConnectionManagerTest, ReceiveThrowsWhenNotConnected) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));

  // Call REAL ReceiveMessage without connecting
  EXPECT_THROW(manager.ReceiveMessage(), std::runtime_error);
}

// ============================================================================
// Test: WssConnectionManager - Disconnect
// ============================================================================

TEST(WssConnectionManagerTest, DisconnectEmitsDisconnectedEvent) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  mock_channel->SetConnectSuccess();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  auto listener = std::make_shared<MockConnectionListener>();
  manager.AddListener(listener);
  
  manager.Connect();

  // Call REAL Disconnect
  manager.Disconnect();

  // Verify OnDisconnected was emitted
  auto events = listener->GetEventSequence();
  EXPECT_TRUE(events.size() > 0);
}

// ============================================================================
// Test: WssConnectionManager - Retry Statistics
// ============================================================================

TEST(WssConnectionManagerTest, RetryStatsTracksRetryCount) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));

  // Call REAL GetRetryStats
  auto stats = manager.GetRetryStats();
  
  EXPECT_EQ(stats.retry_count, 0);
  EXPECT_EQ(stats.max_retries, 5);
  EXPECT_EQ(stats.max_backoff_ms, 8000);
}

// ============================================================================
// Test: WssConnectionManager - Listener Management
// ============================================================================

TEST(WssConnectionManagerTest, CanAddMultipleListeners) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  mock_channel->SetConnectSuccess();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  auto listener1 = std::make_shared<MockConnectionListener>();
  auto listener2 = std::make_shared<MockConnectionListener>();
  
  manager.AddListener(listener1);
  manager.AddListener(listener2);

  // Call REAL Connect
  manager.Connect();

  // Verify both listeners got events
  EXPECT_EQ(listener1->GetOnConnectedCallCount(), 1);
  EXPECT_EQ(listener2->GetOnConnectedCallCount(), 1);
}

TEST(WssConnectionManagerTest, CanRemoveListener) {
  auto mock_channel = std::make_shared<MockWebSocketConnection>();
  mock_channel->SetConnectSuccess();
  
  WssConnectionManager manager(std::static_pointer_cast<IMessageChannel>(mock_channel));
  
  auto listener = std::make_shared<MockConnectionListener>();
  manager.AddListener(listener);
  manager.RemoveListener(listener);

  // Call REAL Connect
  manager.Connect();

  // Verify removed listener didn't receive events
  EXPECT_EQ(listener->GetOnConnectedCallCount(), 0);
}
