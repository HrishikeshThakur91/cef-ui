package com.ui.cef_control.example;

import com.ui.cef_control.CefController;
import com.ui.cef_control.util.Logger;

import java.io.IOException;

/**
 * Example showing how to use CefController from a button click.
 * 
 * This demonstrates the basic workflow:
 * 1. User clicks "Start CEF" button
 * 2. Check if CEF is running
 * 3. If not running, start CEF with initial URL
 * 4. User can then navigate to different URLs
 * 5. User clicks "Stop CEF" button to shutdown
 */
public class ButtonClickExample {

    private CefController cefController;

    /**
     * Initialize the controller with your configuration.
     */
    public void initialize() {
        // Configuration - replace with your actual values
        String cefExePath = "C:\\path\\to\\cef-ui.exe";
        String controlFile = "C:\\temp\\control.dat";
        String controlKey = "dGVzdGtleTEyMzQ1Njc4OTAxMjM0NTY3ODkwMTI="; // Base64 32-byte key

        cefController = new CefController(cefExePath, controlFile, controlKey);
        Logger.info("ButtonClickExample", "CefController initialized");
    }

    /**
     * Called when user clicks "Start CEF" button.
     * 
     * @param startUrl the initial URL to load
     */
    public void onStartButtonClick(String startUrl) {
        Logger.info("ButtonClickExample", "Start button clicked");

        try {
            // Check if already running
            if (cefController.isCefRunning()) {
                Logger.warn("ButtonClickExample", "CEF is already running");
                showMessage("CEF is already running!");
                return;
            }

            // Start CEF
            Logger.info("ButtonClickExample", "Starting CEF with URL: " + startUrl);
            cefController.startCef(startUrl);

            showMessage("CEF started successfully!");

        } catch (IllegalStateException e) {
            Logger.error("ButtonClickExample", "CEF already running: " + e.getMessage());
            showMessage("Error: CEF is already running");

        } catch (IOException e) {
            Logger.error("ButtonClickExample", "Failed to start CEF: " + e.getMessage());
            showMessage("Error: Failed to start CEF - " + e.getMessage());
        }
    }

    /**
     * Called when user clicks "Navigate" button.
     * 
     * @param url the URL to navigate to
     */
    public void onNavigateButtonClick(String url) {
        Logger.info("ButtonClickExample", "Navigate button clicked");

        try {
            // Check if running
            if (!cefController.isCefRunning()) {
                Logger.warn("ButtonClickExample", "CEF is not running");
                showMessage("Please start CEF first!");
                return;
            }

            // Navigate
            Logger.info("ButtonClickExample", "Navigating to: " + url);
            cefController.navigateTo(url);

            showMessage("Navigating to: " + url);

        } catch (IllegalStateException e) {
            Logger.error("ButtonClickExample", "Cannot navigate: " + e.getMessage());
            showMessage("Error: CEF is not running");
        }
    }

    /**
     * Called when user clicks "Stop CEF" button.
     */
    public void onStopButtonClick() {
        Logger.info("ButtonClickExample", "Stop button clicked");

        try {
            // Check if running
            if (!cefController.isCefRunning()) {
                Logger.warn("ButtonClickExample", "CEF is not running");
                showMessage("CEF is not running!");
                return;
            }

            // Stop CEF
            Logger.info("ButtonClickExample", "Stopping CEF...");
            cefController.stopCef();

            showMessage("CEF stopped successfully!");

        } catch (Exception e) {
            Logger.error("ButtonClickExample", "Error stopping CEF: " + e.getMessage());
            showMessage("Error: Failed to stop CEF - " + e.getMessage());
        }
    }

    /**
     * Check CEF status (can be called periodically or on button click).
     * 
     * @return status message
     */
    public String checkStatus() {
        if (cefController.isCefRunning()) {
            return "CEF Status: RUNNING (" + cefController.getStatus() + ")";
        } else {
            return "CEF Status: NOT RUNNING (" + cefController.getStatus() + ")";
        }
    }

    /**
     * Placeholder for showing messages to user (replace with actual UI code).
     */
    private void showMessage(String message) {
        // Replace with actual UI message display
        // e.g., JOptionPane.showMessageDialog(null, message);
        // or update a status label, etc.
        System.out.println("[UI MESSAGE] " + message);
    }

    /**
     * Example main method showing the workflow.
     */
    public static void main(String[] args) {
        ButtonClickExample example = new ButtonClickExample();
        example.initialize();

        // Simulate button clicks
        System.out.println("\n=== Simulating Start Button Click ===");
        example.onStartButtonClick("https://www.google.com");

        // Wait a bit
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        System.out.println("\n=== Checking Status ===");
        System.out.println(example.checkStatus());

        System.out.println("\n=== Simulating Navigate Button Click ===");
        example.onNavigateButtonClick("https://www.github.com");

        // Wait a bit
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        System.out.println("\n=== Simulating Stop Button Click ===");
        example.onStopButtonClick();
    }
}
