#include "pch.h"
#include <gtest/gtest.h>
#include "grpc/UiCommand.h"

using namespace cef_ui::grpc_server;

// ============================================================================
// Test: OpenPageCommand Construction
// ============================================================================

TEST(UiCommandTest, OpenPageCommandConstruction) {
    OpenPageCommand cmd("cmd1", "http://example.com");
    
    EXPECT_EQ(cmd.command_id, "cmd1");
    EXPECT_EQ(cmd.url, "http://example.com");
}

TEST(UiCommandTest, OpenPageCommandWithEmptyFields) {
    OpenPageCommand cmd("", "");
    
    EXPECT_EQ(cmd.command_id, "");
    EXPECT_EQ(cmd.url, "");
}

TEST(UiCommandTest, OpenPageCommandWithSpecialCharacters) {
    OpenPageCommand cmd("cmd-123_test", "https://example.com/path?query=value&foo=bar#fragment");
    
    EXPECT_EQ(cmd.command_id, "cmd-123_test");
    EXPECT_EQ(cmd.url, "https://example.com/path?query=value&foo=bar#fragment");
}

// ============================================================================
// Test: ShutdownCommand Construction
// ============================================================================

TEST(UiCommandTest, ShutdownCommandConstruction) {
    ShutdownCommand cmd;
    // ShutdownCommand has no fields, just verify it constructs
    SUCCEED();
}

// ============================================================================
// Test: UiCommand with OpenPageCommand
// ============================================================================

TEST(UiCommandTest, UiCommandHoldsOpenPageCommand) {
    OpenPageCommand open_cmd("cmd1", "http://example.com");
    UiCommand ui_cmd(std::move(open_cmd));
    
    EXPECT_EQ(ui_cmd.GetType(), CommandType::OPEN_PAGE);
    
    const auto* retrieved = ui_cmd.AsOpenPage();
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->command_id, "cmd1");
    EXPECT_EQ(retrieved->url, "http://example.com");
    
    // AsShutdown should return nullptr
    EXPECT_EQ(ui_cmd.AsShutdown(), nullptr);
}

TEST(UiCommandTest, UiCommandOpenPageAccessors) {
    OpenPageCommand open_cmd("test_id", "http://test.com");
    UiCommand ui_cmd(std::move(open_cmd));
    
    // Verify type check
    EXPECT_EQ(ui_cmd.GetType(), CommandType::OPEN_PAGE);
    
    // Verify correct accessor returns non-null
    EXPECT_NE(ui_cmd.AsOpenPage(), nullptr);
    
    // Verify incorrect accessor returns null
    EXPECT_EQ(ui_cmd.AsShutdown(), nullptr);
}

// ============================================================================
// Test: UiCommand with ShutdownCommand
// ============================================================================

TEST(UiCommandTest, UiCommandHoldsShutdownCommand) {
    ShutdownCommand shutdown_cmd;
    UiCommand ui_cmd(std::move(shutdown_cmd));
    
    EXPECT_EQ(ui_cmd.GetType(), CommandType::SHUTDOWN);
    
    const auto* retrieved = ui_cmd.AsShutdown();
    ASSERT_NE(retrieved, nullptr);
    
    // AsOpenPage should return nullptr
    EXPECT_EQ(ui_cmd.AsOpenPage(), nullptr);
}

TEST(UiCommandTest, UiCommandShutdownAccessors) {
    ShutdownCommand shutdown_cmd;
    UiCommand ui_cmd(std::move(shutdown_cmd));
    
    // Verify type check
    EXPECT_EQ(ui_cmd.GetType(), CommandType::SHUTDOWN);
    
    // Verify correct accessor returns non-null
    EXPECT_NE(ui_cmd.AsShutdown(), nullptr);
    
    // Verify incorrect accessor returns null
    EXPECT_EQ(ui_cmd.AsOpenPage(), nullptr);
}

// ============================================================================
// Test: UiCommand Move Semantics
// ============================================================================

TEST(UiCommandTest, UiCommandMoveConstruction) {
    OpenPageCommand open_cmd("cmd1", "http://example.com");
    UiCommand ui_cmd1(std::move(open_cmd));
    
    // Move construct ui_cmd2 from ui_cmd1
    UiCommand ui_cmd2(std::move(ui_cmd1));
    
    // ui_cmd2 should have the command
    EXPECT_EQ(ui_cmd2.GetType(), CommandType::OPEN_PAGE);
    const auto* retrieved = ui_cmd2.AsOpenPage();
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->command_id, "cmd1");
    EXPECT_EQ(retrieved->url, "http://example.com");
}

TEST(UiCommandTest, UiCommandMoveAssignment) {
    OpenPageCommand open_cmd1("cmd1", "http://example1.com");
    OpenPageCommand open_cmd2("cmd2", "http://example2.com");
    
    UiCommand ui_cmd1(std::move(open_cmd1));
    UiCommand ui_cmd2(std::move(open_cmd2));
    
    // Move assign ui_cmd1 to ui_cmd2
    ui_cmd2 = std::move(ui_cmd1);
    
    // ui_cmd2 should now have cmd1's data
    EXPECT_EQ(ui_cmd2.GetType(), CommandType::OPEN_PAGE);
    const auto* retrieved = ui_cmd2.AsOpenPage();
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->command_id, "cmd1");
    EXPECT_EQ(retrieved->url, "http://example1.com");
}

TEST(UiCommandTest, UiCommandMoveFromDifferentTypes) {
    ShutdownCommand shutdown_cmd;
    UiCommand ui_cmd1(std::move(shutdown_cmd));
    
    OpenPageCommand open_cmd("cmd1", "http://example.com");
    UiCommand ui_cmd2(std::move(open_cmd));
    
    // Move assign SHUTDOWN to OPEN_PAGE
    ui_cmd2 = std::move(ui_cmd1);
    
    // ui_cmd2 should now be SHUTDOWN
    EXPECT_EQ(ui_cmd2.GetType(), CommandType::SHUTDOWN);
    EXPECT_NE(ui_cmd2.AsShutdown(), nullptr);
    EXPECT_EQ(ui_cmd2.AsOpenPage(), nullptr);
}

// ============================================================================
// Test: UiCommand Copy Semantics (should be deleted)
// ============================================================================

// Note: UiCommand should NOT be copyable (only movable)
// This is enforced by the variant holding move-only types
// The following test verifies the type traits

TEST(UiCommandTest, UiCommandIsNotCopyable) {
    // Verify UiCommand is not copy constructible
    EXPECT_FALSE(std::is_copy_constructible<UiCommand>::value);
    
    // Verify UiCommand is not copy assignable
    EXPECT_FALSE(std::is_copy_assignable<UiCommand>::value);
}

TEST(UiCommandTest, UiCommandIsMovable) {
    // Verify UiCommand is move constructible
    EXPECT_TRUE(std::is_move_constructible<UiCommand>::value);
    
    // Verify UiCommand is move assignable
    EXPECT_TRUE(std::is_move_assignable<UiCommand>::value);
}
