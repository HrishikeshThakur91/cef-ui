intellijshowManual,.package com.ui.cef_control;

import com.ui.cef_control.ipc.CefProcessSupervisor;
import com.ui.cef_control.ipc.ControlCommand;
import com.ui.cef_control.ipc.ControlCommandType;
import com.ui.cef_control.ipc.FileEncryptedControlChannel;
import com.ui.cef_control.util.Logger;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * Main controller for CEF browser lifecycle.
 * 
 * Provides simple API for starting/stopping CEF and sending commands.
 * Thread-safe for basic operations.
 * 
 * Usage:
 * 
 * <pre>
 * CefController controller = new CefController(cefExePath, controlFile, controlKey);
 * controller.startCef(startUrl);
 * controller.navigateTo(url);
 * controller.stopCef();
 * </pre>
 */
public class CefController {

    private final String cefExecutablePath;
    private final Path controlFile;
    private final String controlKey;

    private CefProcessSupervisor processSupervisor;
    private FileEncryptedControlChannel controlChannel;
    private volatile boolean initialized = false;

    /**
     * Creates a new CEF controller.
     * 
     * @param cefExecutablePath path to CEF executable
     * @param controlFilePath   path to control file for commands
     * @param controlKey        Base64-encoded AES-256 key
     */
    public CefController(String cefExecutablePath, String controlFilePath, String controlKey) {
        this.cefExecutablePath = cefExecutablePath;
        this.controlFile = Paths.get(controlFilePath);
        this.controlKey = controlKey;
    }

    /**
     * Starts the CEF process if not already running.
     * 
     * Workflow:
     * 1. Check if CEF is already running
     * 2. If not, initialize control channel
     * 3. Start CEF process with control file/key arguments
     * 4. Send START command
     * 5. Send NAVIGATE command with URL
     * 
     * @param startUrl the URL to load on startup
     * @throws IllegalStateException if already running
     * @throws IOException           if process start fails
     */
    public synchronized void startCef(String startUrl) throws IOException {
        Logger.info("CefController", "startCef() called with URL: " + startUrl);

        // Step 1: Check if already running
        if (isCefRunning()) {
            Logger.warn("CefController", "CEF is already running, ignoring start request");
            throw new IllegalStateException("CEF is already running");
        }

        try {
            // Step 2: Initialize control channel
            Logger.info("CefController", "Initializing control channel...");
            controlChannel = new FileEncryptedControlChannel(controlFile, controlKey);

            // Step 3: Initialize and start process supervisor
            Logger.info("CefController", "Starting CEF process...");
            processSupervisor = new CefProcessSupervisor(
                    cefExecutablePath,
                    startUrl,
                    controlFile,
                    controlKey);
            processSupervisor.start();

            // Wait a bit for process to initialize
            Thread.sleep(1000);

            // Step 4: Send START command
            Logger.info("CefController", "Sending START command...");
            sendCommand(ControlCommandType.Start, null);

            // Step 5: Send NAVIGATE command
            Logger.info("CefController", "Sending NAVIGATE command...");
            Map<String, String> payload = new HashMap<>();
            payload.put("url", startUrl);
            sendCommand(ControlCommandType.Navigate, payload);

            initialized = true;
            Logger.info("CefController", "CEF started successfully");

        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            Logger.error("CefController", "Interrupted during startup: " + e.getMessage());
            cleanup();
            throw new IOException("CEF startup interrupted", e);
        } catch (Exception e) {
            Logger.error("CefController", "Failed to start CEF: " + e.getMessage());
            cleanup();
            throw new IOException("Failed to start CEF", e);
        }
    }

    /**
     * Navigates to a new URL.
     * 
     * @param url the URL to navigate to
     * @throws IllegalStateException if CEF is not running
     */
    public void navigateTo(String url) {
        Logger.info("CefController", "navigateTo() called with URL: " + url);

        if (!isCefRunning()) {
            Logger.error("CefController", "Cannot navigate: CEF is not running");
            throw new IllegalStateException("CEF is not running");
        }

        Map<String, String> payload = new HashMap<>();
        payload.put("url", url);
        sendCommand(ControlCommandType.Navigate, payload);

        Logger.info("CefController", "Navigate command sent");
    }

    /**
     * Stops the CEF process gracefully.
     */
    public synchronized void stopCef() {
        Logger.info("CefController", "stopCef() called");

        if (!isCefRunning()) {
            Logger.warn("CefController", "CEF is not running, nothing to stop");
            return;
        }

        try {
            // Send shutdown command
            Logger.info("CefController", "Sending SHUTDOWN command...");
            sendCommand(ControlCommandType.Shutdown, null);

            // Wait a bit for graceful shutdown
            Thread.sleep(500);

            // Stop process supervisor
            if (processSupervisor != null) {
                processSupervisor.stop();
            }

            Logger.info("CefController", "CEF stopped successfully");

        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            Logger.error("CefController", "Interrupted during shutdown: " + e.getMessage());
        } finally {
            cleanup();
        }
    }

    /**
     * Checks if CEF process is currently running.
     * 
     * @return true if running, false otherwise
     */
    public boolean isCefRunning() {
        return processSupervisor != null && processSupervisor.isAlive();
    }

    /**
     * Gets the current process status.
     * 
     * @return the process status, or NOT_STARTED if not initialized
     */
    public CefProcessSupervisor.ProcessStatus getStatus() {
        if (processSupervisor == null) {
            return CefProcessSupervisor.ProcessStatus.NOT_STARTED;
        }
        return processSupervisor.getStatus();
    }

    /**
     * Sends a command to the CEF process.
     * 
     * @param type    the command type
     * @param payload optional payload (can be null)
     */
    private void sendCommand(ControlCommandType type, Map<String, String> payload) {
        if (controlChannel == null) {
            Logger.error("CefController", "Control channel not initialized");
            throw new IllegalStateException("Control channel not initialized");
        }

        String commandId = UUID.randomUUID().toString();
        ControlCommand command = new ControlCommand(commandId, type, payload);

        try {
            controlChannel.sendCommand(command);
        } catch (Exception e) {
            Logger.error("CefController", "Failed to send command: " + e.getMessage());
            throw new RuntimeException("Failed to send command", e);
        }
    }

    /**
     * Cleans up resources.
     */
    private void cleanup() {
        if (controlChannel != null) {
            controlChannel.shutdown();
            controlChannel = null;
        }
        processSupervisor = null;
        initialized = false;
    }
}
