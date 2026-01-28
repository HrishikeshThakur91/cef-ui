#include "pch.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include "grpc/CommandQueue.h"
#include "grpc/UiCommand.h"

using namespace cef_ui::grpc_server;

// ============================================================================
// Test: CommandQueue Basic Operations
// ============================================================================

TEST(CommandQueueTest, EnqueueDequeueOpenPageCommand) {
    CommandQueue queue;
    OpenPageCommand cmd("cmd1", "http://example.com");
    queue.Enqueue(UiCommand(std::move(cmd)));
    
    auto result = queue.Dequeue();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->GetType(), CommandType::OPEN_PAGE);
    
    const auto* open_page = result->AsOpenPage();
    ASSERT_NE(open_page, nullptr);
    EXPECT_EQ(open_page->command_id, "cmd1");
    EXPECT_EQ(open_page->url, "http://example.com");
}

TEST(CommandQueueTest, EnqueueDequeueShutdownCommand) {
    CommandQueue queue;
    ShutdownCommand cmd;
    queue.Enqueue(UiCommand(std::move(cmd)));
    
    auto result = queue.Dequeue();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->GetType(), CommandType::SHUTDOWN);
    
    const auto* shutdown = result->AsShutdown();
    ASSERT_NE(shutdown, nullptr);
}

TEST(CommandQueueTest, DequeueEmptyQueueReturnsNullopt) {
    CommandQueue queue;
    auto result = queue.Dequeue();
    EXPECT_FALSE(result.has_value());
}

TEST(CommandQueueTest, FIFOOrdering) {
    CommandQueue queue;
    
    // Enqueue multiple commands
    queue.Enqueue(UiCommand(OpenPageCommand("cmd1", "http://example1.com")));
    queue.Enqueue(UiCommand(OpenPageCommand("cmd2", "http://example2.com")));
    queue.Enqueue(UiCommand(OpenPageCommand("cmd3", "http://example3.com")));
    
    // Dequeue and verify order
    auto result1 = queue.Dequeue();
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->AsOpenPage()->command_id, "cmd1");
    
    auto result2 = queue.Dequeue();
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->AsOpenPage()->command_id, "cmd2");
    
    auto result3 = queue.Dequeue();
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3->AsOpenPage()->command_id, "cmd3");
    
    // Queue should be empty now
    auto result4 = queue.Dequeue();
    EXPECT_FALSE(result4.has_value());
}

TEST(CommandQueueTest, MultipleEnqueueDequeue) {
    CommandQueue queue;
    
    // Enqueue, dequeue, enqueue, dequeue pattern
    queue.Enqueue(UiCommand(OpenPageCommand("cmd1", "http://example1.com")));
    auto result1 = queue.Dequeue();
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->AsOpenPage()->command_id, "cmd1");
    
    queue.Enqueue(UiCommand(OpenPageCommand("cmd2", "http://example2.com")));
    queue.Enqueue(UiCommand(ShutdownCommand()));
    
    auto result2 = queue.Dequeue();
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->GetType(), CommandType::OPEN_PAGE);
    
    auto result3 = queue.Dequeue();
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3->GetType(), CommandType::SHUTDOWN);
}

// ============================================================================
// Test: CommandQueue Thread Safety
// ============================================================================

TEST(CommandQueueTest, ThreadSafetyConcurrentEnqueue) {
    CommandQueue queue;
    const int num_threads = 10;
    const int commands_per_thread = 100;
    std::atomic<int> enqueue_count{0};
    
    std::vector<std::thread> threads;
    
    // Multiple threads enqueue concurrently
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&queue, &enqueue_count, i, commands_per_thread]() {
            for (int j = 0; j < commands_per_thread; ++j) {
                std::string cmd_id = "thread" + std::to_string(i) + "_cmd" + std::to_string(j);
                queue.Enqueue(UiCommand(OpenPageCommand(cmd_id, "http://example.com")));
                enqueue_count++;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all commands were enqueued
    EXPECT_EQ(enqueue_count.load(), num_threads * commands_per_thread);
    
    // Dequeue all and verify count
    int dequeue_count = 0;
    while (queue.Dequeue().has_value()) {
        dequeue_count++;
    }
    
    EXPECT_EQ(dequeue_count, num_threads * commands_per_thread);
}

TEST(CommandQueueTest, ThreadSafetyConcurrentEnqueueDequeue) {
    CommandQueue queue;
    const int num_producer_threads = 5;
    const int num_consumer_threads = 5;
    const int commands_per_producer = 100;
    
    std::atomic<int> enqueue_count{0};
    std::atomic<int> dequeue_count{0};
    std::atomic<bool> producers_done{false};
    
    std::vector<std::thread> threads;
    
    // Producer threads
    for (int i = 0; i < num_producer_threads; ++i) {
        threads.emplace_back([&queue, &enqueue_count, i, commands_per_producer]() {
            for (int j = 0; j < commands_per_producer; ++j) {
                std::string cmd_id = "producer" + std::to_string(i) + "_cmd" + std::to_string(j);
                queue.Enqueue(UiCommand(OpenPageCommand(cmd_id, "http://example.com")));
                enqueue_count++;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    // Consumer threads
    for (int i = 0; i < num_consumer_threads; ++i) {
        threads.emplace_back([&queue, &dequeue_count, &producers_done]() {
            while (!producers_done.load() || queue.Dequeue().has_value()) {
                if (auto cmd = queue.Dequeue()) {
                    dequeue_count++;
                } else {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
        });
    }
    
    // Wait for producers to finish
    for (int i = 0; i < num_producer_threads; ++i) {
        threads[i].join();
    }
    producers_done = true;
    
    // Wait for consumers to finish
    for (int i = num_producer_threads; i < threads.size(); ++i) {
        threads[i].join();
    }
    
    // Verify all commands were processed
    EXPECT_EQ(enqueue_count.load(), num_producer_threads * commands_per_producer);
    EXPECT_EQ(dequeue_count.load(), num_producer_threads * commands_per_producer);
}

TEST(CommandQueueTest, ThreadSafetyNoDataRaces) {
    // This test verifies that concurrent operations don't cause crashes or data corruption
    // Run with thread sanitizer for best results
    CommandQueue queue;
    std::atomic<bool> stop{ false };

    std::thread producer([&queue, &stop]() {
        int counter = 0;
        while (!stop.load()) {
            queue.Enqueue(UiCommand(OpenPageCommand("cmd" + std::to_string(counter++), "http://example.com")));
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        });

    std::thread consumer([&queue, &stop]() {
        while (!stop.load()) {
            queue.Dequeue();
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        });

    // Run for a short duration
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop = true;

    producer.join();
    consumer.join();

    // If we reach here without crashing, the test passes
    SUCCEED();
}
