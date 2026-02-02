package com.ui.cef_control;

import com.ui.cef_control.grpc.GrpcIpcServer;
import com.ui.cef_control.http.VuePressHttpServer;
import com.ui.cef_control.http.HttpServerConfig;
import java.io.IOException;

/**
 * Main application manager for CEF control infrastructure.
 * 
 * Manages lifecycle of:
 * - VuePress HTTP server (documentation)
 * - gRPC IPC server (CEF communication)
 * - CEF browser process
 * 
 * Usage:
 * 1. Call initializeServers() at application startup
 * 2. Call launchCefBrowser() on button click
 * 3. Call shutdown() on application exit
 */
public class CefApplicationManager {

    private GrpcIpcServer grpcServer;
    private VuePressHttpServer httpServer;
    private CefLauncher cefLauncher;
    private boolean initialized = false;

    /**
     * ENTRY POINT #1: Initialize servers at application startup.
     * 
     * Starts both VuePress HTTP server and gRPC server.
     * Must be called before launchCefBrowser().
     * 
     * @param docsPath Path to VuePress documentation build output
     * @param httpPort HTTP server port (e.g., 8080)
     * @param grpcPort gRPC server port (e.g., 50051)
     * @throws IOException           if servers fail to start
     * @throws IllegalStateException if already initialized
     */
    public void initializeServers(String docsPath, int httpPort, int grpcPort)
            throws IOException {

        if (initialized) {
            System.out.println("[CefApp] Servers already initialized");
            return;
        }

        System.out.println("[CefApp] Initializing CEF infrastructure...");

        // Start HTTP server for documentation
        HttpServerConfig httpConfig = new HttpServerConfig(
                docsPath,
                httpPort,
                "127.0.0.1" // localhost only
        );
        httpServer = new VuePressHttpServer(httpConfig);
        httpServer.start();

        System.out.println("[CefApp] HTTP server started: " + httpServer.getActualAddress());

        // Start gRPC server for CEF communication
        grpcServer = new GrpcIpcServer(grpcPort);
        grpcServer.start();

        System.out.println("[CefApp] gRPC server started on port: " + grpcServer.getPort());

        // Create CEF launcher (but don't launch yet)
        cefLauncher = new CefLauncher();

        initialized = true;
        System.out.println("[CefApp] Infrastructure ready");
    }

    /**
     * ENTRY POINT #2: Launch CEF browser on button click.
     * 
     * Launches CEF browser window with specified URL.
     * Servers must be initialized first.
     * 
     * @param url URL to load in CEF browser
     * @throws IOException           if CEF process fails to start
     * @throws IllegalStateException if servers not initialized
     */
    public void launchCefBrowser(String url) throws IOException {
        if (!initialized) {
            throw new IllegalStateException(
                    "Servers not initialized. Call initializeServers() first.");
        }

        if (cefLauncher.isRunning()) {
            System.out.println("[CefApp] CEF already running");
            return;
        }

        System.out.println("[CefApp] Launching CEF browser with URL: " + url);
        cefLauncher.launchCef(url);
    }

    /**
     * ENTRY POINT #3: Shutdown all infrastructure.
     * 
     * Stops CEF process, gRPC server, and HTTP server.
     * Call on application shutdown or when CEF is no longer needed.
     */
    public void shutdown() {
        System.out.println("[CefApp] Shutting down CEF infrastructure...");

        // Stop CEF process
        if (cefLauncher != null) {
            try {
                cefLauncher.shutdown();
            } catch (Exception e) {
                System.err.println("[CefApp] Error shutting down CEF: " + e.getMessage());
            }
        }

        // Stop gRPC server
        if (grpcServer != null && grpcServer.isRunning()) {
            grpcServer.stop();
        }

        // Stop HTTP server
        if (httpServer != null && httpServer.isRunning()) {
            httpServer.stop();
        }

        initialized = false;
        System.out.println("[CefApp] Shutdown complete");
    }

    /**
     * Check if infrastructure is initialized and ready.
     * 
     * @return true if servers are running
     */
    public boolean isInitialized() {
        return initialized;
    }

    /**
     * Check if CEF browser is currently running.
     * 
     * @return true if CEF process is active
     */
    public boolean isCefRunning() {
        return cefLauncher != null && cefLauncher.isRunning();
    }

    /**
     * Get the HTTP server URL.
     * 
     * @return HTTP server URL (e.g., "http://127.0.0.1:8080") or null if not
     *         running
     */
    public String getHttpServerUrl() {
        if (httpServer != null && httpServer.isRunning()) {
            return "http://" + httpServer.getActualAddress();
        }
        return null;
    }

    /**
     * Get the gRPC server port.
     * 
     * @return gRPC port number or -1 if not running
     */
    public int getGrpcPort() {
        if (grpcServer != null && grpcServer.isRunning()) {
            return grpcServer.getPort();
        }
        return -1;
    }
}
