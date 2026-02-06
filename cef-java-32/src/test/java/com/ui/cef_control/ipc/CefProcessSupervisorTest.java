package com.ui.cef_control.ipc;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Test for CefProcessSupervisor.
 * 
 * Uses mock processes (system commands) to verify supervisor logic
 * since we don't have an actual CEF executable for testing.
 */
class CefProcessSupervisorTest {

    @TempDir
    Path tempDir;

    private Path controlFile;
    private String controlKey;
    private CefProcessSupervisor supervisor;

    @BeforeEach
    void setUp() {
        controlFile = tempDir.resolve("control.dat");
        controlKey = "dGVzdGtleTEyMzQ1Njc4OTAxMjM0NTY3ODkwMTI="; // 32-byte key in base64
    }

    @AfterEach
    void tearDown() {
        if (supervisor != null && supervisor.isAlive()) {
            supervisor.stop();
        }
    }

    @Test
    void testConstructorValidatesNullExecutablePath() {
        assertThrows(NullPointerException.class, () -> {
            new CefProcessSupervisor(null, "http://test.com", controlFile, controlKey);
        });
    }

    @Test
    void testConstructorValidatesNullStartUrl() {
        assertThrows(NullPointerException.class, () -> {
            new CefProcessSupervisor("cef.exe", null, controlFile, controlKey);
        });
    }

    @Test
    void testConstructorValidatesNullControlFile() {
        assertThrows(NullPointerException.class, () -> {
            new CefProcessSupervisor("cef.exe", "http://test.com", null, controlKey);
        });
    }

    @Test
    void testConstructorValidatesNullControlKey() {
        assertThrows(NullPointerException.class, () -> {
            new CefProcessSupervisor("cef.exe", "http://test.com", controlFile, null);
        });
    }

    @Test
    void testInitialStatusIsNotStarted() {
        supervisor = new CefProcessSupervisor("cef.exe", "http://test.com", controlFile, controlKey);
        assertEquals(CefProcessSupervisor.ProcessStatus.NOT_STARTED, supervisor.getStatus());
        assertFalse(supervisor.isAlive());
        assertNull(supervisor.getExitCode());
    }

    @Test
    void testStartProcessSuccessfully() throws Exception {
        // Use a simple command that will run briefly
        String command = isWindows() ? "cmd.exe /c echo test" : "echo test";
        supervisor = new CefProcessSupervisor(command, "http://test.com", controlFile, controlKey);

        supervisor.start();

        // Process should be running or have just finished
        assertTrue(supervisor.getStatus() == CefProcessSupervisor.ProcessStatus.RUNNING ||
                supervisor.getStatus() == CefProcessSupervisor.ProcessStatus.STOPPED);
    }

    @Test
    void testCannotStartWhenAlreadyRunning() throws Exception {
        // Use a long-running command
        String command = isWindows() ? "cmd.exe /c timeout /t 10" : "sleep 10";
        supervisor = new CefProcessSupervisor(command, "http://test.com", controlFile, controlKey);

        supervisor.start();
        assertTrue(supervisor.isAlive());

        // Try to start again
        assertThrows(IllegalStateException.class, () -> {
            supervisor.start();
        });
    }

    @Test
    void testStopProcessGracefully() throws Exception {
        // Use a long-running command
        String command = isWindows() ? "cmd.exe /c timeout /t 30" : "sleep 30";
        supervisor = new CefProcessSupervisor(command, "http://test.com", controlFile, controlKey);

        supervisor.start();
        assertTrue(supervisor.isAlive());
        assertEquals(CefProcessSupervisor.ProcessStatus.RUNNING, supervisor.getStatus());

        supervisor.stop();

        assertFalse(supervisor.isAlive());
        assertEquals(CefProcessSupervisor.ProcessStatus.STOPPED, supervisor.getStatus());
        assertNotNull(supervisor.getExitCode());
    }

    @Test
    void testStopWhenNotRunningIsNoOp() {
        supervisor = new CefProcessSupervisor("cef.exe", "http://test.com", controlFile, controlKey);

        // Should not throw
        supervisor.stop();

        assertEquals(CefProcessSupervisor.ProcessStatus.NOT_STARTED, supervisor.getStatus());
    }

    @Test
    void testDetectNormalExit() throws Exception {
        // Use a command that exits with code 0
        String command = isWindows() ? "cmd.exe /c exit 0" : "sh -c 'exit 0'";
        supervisor = new CefProcessSupervisor(command, "http://test.com", controlFile, controlKey);

        supervisor.start();

        // Wait for process to exit
        Thread.sleep(1000);

        assertFalse(supervisor.isAlive());
        assertEquals(CefProcessSupervisor.ProcessStatus.STOPPED, supervisor.getStatus());
        assertEquals(0, supervisor.getExitCode());
    }

    @Test
    void testDetectCrash() throws Exception {
        // Use a command that exits with non-zero code
        String command = isWindows() ? "cmd.exe /c exit 1" : "sh -c 'exit 1'";
        supervisor = new CefProcessSupervisor(command, "http://test.com", controlFile, controlKey);

        supervisor.start();

        // Wait for process to exit
        Thread.sleep(1000);

        assertFalse(supervisor.isAlive());
        assertEquals(CefProcessSupervisor.ProcessStatus.CRASHED, supervisor.getStatus());
        assertEquals(1, supervisor.getExitCode());
    }

    @Test
    void testInvalidExecutableThrowsIOException() {
        supervisor = new CefProcessSupervisor("nonexistent-executable-12345", "http://test.com", controlFile,
                controlKey);

        assertThrows(IOException.class, () -> {
            supervisor.start();
        });

        assertEquals(CefProcessSupervisor.ProcessStatus.NOT_STARTED, supervisor.getStatus());
        assertFalse(supervisor.isAlive());
    }

    @Test
    void testIsAliveReflectsProcessState() throws Exception {
        String command = isWindows() ? "cmd.exe /c timeout /t 5" : "sleep 5";
        supervisor = new CefProcessSupervisor(command, "http://test.com", controlFile, controlKey);

        assertFalse(supervisor.isAlive());

        supervisor.start();
        assertTrue(supervisor.isAlive());

        supervisor.stop();
        assertFalse(supervisor.isAlive());
    }

    @Test
    void testExitCodeCapturedOnCrash() throws Exception {
        // Use a command that exits with specific code
        String command = isWindows() ? "cmd.exe /c exit 42" : "sh -c 'exit 42'";
        supervisor = new CefProcessSupervisor(command, "http://test.com", controlFile, controlKey);

        supervisor.start();

        // Wait for process to exit
        Thread.sleep(1000);

        assertEquals(42, supervisor.getExitCode());
        assertEquals(CefProcessSupervisor.ProcessStatus.CRASHED, supervisor.getStatus());
    }

    /**
     * Helper to detect Windows OS.
     */
    private boolean isWindows() {
        return System.getProperty("os.name").toLowerCase().contains("win");
    }
}
