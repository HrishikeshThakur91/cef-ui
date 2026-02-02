package com.ui.cef_control;

import com.ui.cef_control.grpc.CefServiceBootstrap;
import java.io.IOException;

/**
 * Entry point for launching CEF browser from UI button click.
 * 
 * Usage:
 * <pre>
 * CefLauncher launcher = new CefLauncher();
 * launcher.launchCef("http://localhost:8080/docs");
 * // ... later ...
 * launcher.shutdown();
 * </pre>
 * 
 * This class manages the lifecycle of the CEF browser process and
 * the gRPC communication channel.
 */
public class CefLauncher {
    
    private CefServiceBootstrap bootstrap;
    private Thread cefThread;
    private volatile boolean isRunning = false;
    
    /**
     * Launch CEF browser with the specified URL.
     * Call this method from your button click handler.
     * 
     * This method is non-blocking - it starts CEF in a background thread.
     * 
     * @param url The URL to open in the CEF browser
     * @throws IOException if CEF process cannot be started
     * @throws IllegalStateException if CEF is already running
     */
    public void launchCef(String url) throws IOException {
        if (isRunning) {
            throw new IllegalStateException("CEF is already running. Call shutdown() first.");
        }
        
        // Configuration
        // TODO: Make these configurable via AppConfig or properties file
        int ipcPort = 50051;
        String sessionToken = "session-" + System.currentTimeMillis();
        
        System.out.println("[CefLauncher] Launching CEF browser...");
        System.out.println("[CefLauncher] URL: " + url);
        System.out.println("[CefLauncher] IPC Port: " + ipcPort);
        System.out.println("[CefLauncher] Session Token: " + sessionToken);
        
        // Create bootstrap
        bootstrap = new CefServiceBootstrap(ipcPort, sessionToken, url);
        
        // Run in background thread to avoid blocking UI
        cefThread = new Thread(() -> {
            try {
                isRunning = true;
                bootstrap.run();
            } catch (Exception e) {
                System.err.println("[CefLauncher] CEF bootstrap failed: " + e.getMessage());
                e.printStackTrace();
            } finally {
                isRunning = false;
            }
        });
        
        cefThread.setName("CEF-Bootstrap");
        cefThread.setDaemon(false);  // Keep JVM alive while CEF is running
        cefThread.start();
        
        System.out.println("[CefLauncher] CEF bootstrap thread started");
    }
    
    /**
     * Shutdown CEF gracefully.
     * Blocks until CEF process terminates or timeout expires.
     * 
     * @throws InterruptedException if shutdown is interrupted
     */
    public void shutdown() throws InterruptedException {
        if (!isRunning) {
            System.out.println("[CefLauncher] CEF is not running, nothing to shutdown");
            return;
        }
        
        System.out.println("[CefLauncher] Shutting down CEF...");
        
        if (bootstrap != null) {
            bootstrap.shutdown();
        }
        
        if (cefThread != null) {
            // Wait up to 5 seconds for graceful shutdown
            cefThread.join(5000);
            
            if (cefThread.isAlive()) {
                System.err.println("[CefLauncher] WARNING: CEF thread did not terminate within timeout");
                // Force interrupt
                cefThread.interrupt();
            } else {
                System.out.println("[CefLauncher] CEF shutdown complete");
            }
        }
        
        isRunning = false;
    }
    
    /**
     * Check if CEF is currently running.
     * 
     * @return true if CEF browser is running
     */
    public boolean isRunning() {
        return isRunning;
    }
    
    /**
     * Example usage for testing.
     */
    public static void main(String[] args) {
        CefLauncher launcher = new CefLauncher();
        
        try {
            // Launch CEF with a test URL
            String url = args.length > 0 ? args[0] : "http://localhost:8080/docs";
            launcher.launchCef(url);
            
            System.out.println("CEF browser launched. Press Enter to shutdown...");
            System.in.read();
            
            // Shutdown
            launcher.shutdown();
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
