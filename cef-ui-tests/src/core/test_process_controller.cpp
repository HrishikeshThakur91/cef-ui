#pragma once
#include "pch.h"

#include <memory>
#include <vector>
#include <gtest/gtest.h>

#include "core/ProcessController.h"
#include "core/ILifecycleListener.h"

using namespace cef_ui::core;

// ============================================================================
// Mock Listener for testing event emission
// ============================================================================

class MockLifecycleListener : public ILifecycleListener {
 public:
  MockLifecycleListener() = default;

  void OnStarted() override {
    started_called_ = true;
    event_sequence_.push_back("OnStarted");
  }

  void OnStopping() override {
    stopping_called_ = true;
    event_sequence_.push_back("OnStopping");
  }

  void OnStopped() override {
    stopped_called_ = true;
    event_sequence_.push_back("OnStopped");
  }

  void OnError(const std::string& error_message) override {
    error_called_ = true;
    error_message_ = error_message;
    event_sequence_.push_back("OnError:" + error_message);
  }

  bool started_called_ = false;
  bool stopping_called_ = false;
  bool stopped_called_ = false;
  bool error_called_ = false;
  std::string error_message_;
  std::vector<std::string> event_sequence_;

  void Reset() {
    started_called_ = false;
    stopping_called_ = false;
    stopped_called_ = false;
    error_called_ = false;
    error_message_.clear();
    event_sequence_.clear();
  }
};

// ============================================================================
// Test: Initial State
// ============================================================================

TEST(ProcessControllerTest, InitialStateIsIdle) {
  ProcessController controller;
  EXPECT_EQ(controller.GetState(), ProcessState::Idle);
}

// ============================================================================
// Test: Start Transitions State Correctly
// ============================================================================

TEST(ProcessControllerTest, StartTransitionsToStarted) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();

  EXPECT_EQ(controller.GetState(), ProcessState::Started);
}

TEST(ProcessControllerTest, StartEmitsStartedEvent) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();

  EXPECT_TRUE(listener->started_called_);
}

TEST(ProcessControllerTest, StartEventIsEmittedToMultipleListeners) {
  ProcessController controller;
  auto listener1 = std::make_shared<MockLifecycleListener>();
  auto listener2 = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener1);
  controller.AddListener(listener2);
  controller.Start();

  EXPECT_TRUE(listener1->started_called_);
  EXPECT_TRUE(listener2->started_called_);
}

TEST(ProcessControllerTest, StartWithoutListenerSucceeds) {
  ProcessController controller;
  // No listener added
  controller.Start();
  EXPECT_EQ(controller.GetState(), ProcessState::Started);
}

// ============================================================================
// Test: Shutdown Transitions State Correctly
// ============================================================================

TEST(ProcessControllerTest, ShutdownTransitionsToStopped) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();
  controller.Shutdown();

  EXPECT_EQ(controller.GetState(), ProcessState::Stopped);
}

TEST(ProcessControllerTest, ShutdownEmitsStoppingAndStoppedEvents) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();
  listener->Reset();  // Clear Start event
  
  controller.Shutdown();

  EXPECT_TRUE(listener->stopping_called_);
  EXPECT_TRUE(listener->stopped_called_);
}

TEST(ProcessControllerTest, ShutdownEventOrderIsCorrect) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();
  listener->Reset();  // Clear Start event
  
  controller.Shutdown();

  EXPECT_EQ(listener->event_sequence_.size(), 2);
  EXPECT_EQ(listener->event_sequence_[0], "OnStopping");
  EXPECT_EQ(listener->event_sequence_[1], "OnStopped");
}

TEST(ProcessControllerTest, ShutdownEmitsEventsToMultipleListeners) {
  ProcessController controller;
  auto listener1 = std::make_shared<MockLifecycleListener>();
  auto listener2 = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener1);
  controller.AddListener(listener2);
  
  controller.Start();
  listener1->Reset();
  listener2->Reset();
  
  controller.Shutdown();

  EXPECT_TRUE(listener1->stopping_called_);
  EXPECT_TRUE(listener1->stopped_called_);
  EXPECT_TRUE(listener2->stopping_called_);
  EXPECT_TRUE(listener2->stopped_called_);
}

TEST(ProcessControllerTest, ShutdownWithoutListenerSucceeds) {
  ProcessController controller;
  controller.Start();
  controller.Shutdown();
  EXPECT_EQ(controller.GetState(), ProcessState::Stopped);
}

// ============================================================================
// Test: Double Start is Rejected
// ============================================================================

TEST(ProcessControllerTest, DoubleStartThrowsException) {
  ProcessController controller;
  controller.Start();
  EXPECT_THROW(controller.Start(), std::logic_error);
}

TEST(ProcessControllerTest, DoubleStartLeavesStateUnchanged) {
  ProcessController controller;
  controller.Start();
  
  try {
    controller.Start();
  } catch (const std::logic_error&) {
    // Expected
  }

  EXPECT_EQ(controller.GetState(), ProcessState::Started);
}

TEST(ProcessControllerTest, DoubleStartDoesNotEmitDuplicateEvents) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();
  int event_count_after_start = listener->event_sequence_.size();
  
  try {
    controller.Start();
  } catch (const std::logic_error&) {
    // Expected
  }

  // No additional events should be emitted
  EXPECT_EQ(listener->event_sequence_.size(), event_count_after_start);
}

// ============================================================================
// Test: Lifecycle Event Sequence
// ============================================================================

TEST(ProcessControllerTest, FullLifecycleEventSequenceIsCorrect) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();
  controller.Shutdown();

  EXPECT_EQ(listener->event_sequence_.size(), 3);
  EXPECT_EQ(listener->event_sequence_[0], "OnStarted");
  EXPECT_EQ(listener->event_sequence_[1], "OnStopping");
  EXPECT_EQ(listener->event_sequence_[2], "OnStopped");
}

TEST(ProcessControllerTest, ListenerCanBeAddedAfterStart) {
  ProcessController controller;
  auto listener1 = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener1);
  controller.Start();
  listener1->Reset();

  // Add a second listener after start
  auto listener2 = std::make_shared<MockLifecycleListener>();
  controller.AddListener(listener2);
  
  controller.Shutdown();

  // Both should receive shutdown events
  EXPECT_TRUE(listener1->stopping_called_);
  EXPECT_TRUE(listener1->stopped_called_);
  EXPECT_TRUE(listener2->stopping_called_);
  EXPECT_TRUE(listener2->stopped_called_);
}

// ============================================================================
// Test: Remove Listener
// ============================================================================

TEST(ProcessControllerTest, RemovedListenerDoesNotReceiveEvents) {
  ProcessController controller;
  auto listener1 = std::make_shared<MockLifecycleListener>();
  auto listener2 = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener1);
  controller.AddListener(listener2);
  
  controller.RemoveListener(listener1);
  controller.Start();

  // listener1 should not receive the event
  EXPECT_FALSE(listener1->started_called_);
  // listener2 should receive it
  EXPECT_TRUE(listener2->started_called_);
}

TEST(ProcessControllerTest, RemoveListenerTwiceDoesNotThrow) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.RemoveListener(listener);
  
  // Should not throw
  EXPECT_NO_THROW(controller.RemoveListener(listener));
}

// ============================================================================
// Test: State Queries
// ============================================================================

TEST(ProcessControllerTest, StateAfterShutdownIsStopped) {
  ProcessController controller;
  controller.Start();
  controller.Shutdown();
  EXPECT_EQ(controller.GetState(), ProcessState::Stopped);
}

TEST(ProcessControllerTest, MultipleStateQueriesReturnConsistentState) {
  ProcessController controller;
  controller.Start();
  
  ProcessState state1 = controller.GetState();
  ProcessState state2 = controller.GetState();
  
  EXPECT_EQ(state1, state2);
  EXPECT_EQ(state1, ProcessState::Started);
}

// ============================================================================
// Test: Idempotent Shutdown
// ============================================================================

TEST(ProcessControllerTest, ShutdownOnAlreadyStoppedIsIdempotent) {
  ProcessController controller;
  controller.Start();
  controller.Shutdown();
  
  // Second shutdown should not throw
  EXPECT_NO_THROW(controller.Shutdown());
  EXPECT_EQ(controller.GetState(), ProcessState::Stopped);
}

TEST(ProcessControllerTest, MultipleShutdownsDoNotEmitMultipleEvents) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Start();
  listener->Reset();
  
  controller.Shutdown();
  int event_count_after_first_shutdown = listener->event_sequence_.size();
  listener->Reset();
  
  controller.Shutdown();
  
  // No additional events should be emitted on second shutdown
  EXPECT_EQ(listener->event_sequence_.size(), 0);
}

// ============================================================================
// Test: State Before Start
// ============================================================================

TEST(ProcessControllerTest, ShutdownBeforeStartIsIdempotent) {
  ProcessController controller;
  EXPECT_NO_THROW(controller.Shutdown());
  EXPECT_EQ(controller.GetState(), ProcessState::Stopped);
}

TEST(ProcessControllerTest, ShutdownBeforeStartDoesNotEmitEvents) {
  ProcessController controller;
  auto listener = std::make_shared<MockLifecycleListener>();
  
  controller.AddListener(listener);
  controller.Shutdown();

  EXPECT_FALSE(listener->started_called_);
  EXPECT_FALSE(listener->stopping_called_);
  EXPECT_FALSE(listener->stopped_called_);
  EXPECT_FALSE(listener->error_called_);
}
