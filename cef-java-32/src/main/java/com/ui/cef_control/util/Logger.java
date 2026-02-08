package com.ui.cef_control.util;

/**
 * Simple logging utility for the CEF control system.
 * 
 * Provides static methods for logging at different severity levels.
 * Thread-safe implementation using synchronized methods.
 * 
 * Severity levels:
 * - INFO: Lifecycle events, command sent, startup/shutdown
 * - WARN: Recoverable issues, ignored commands, validation failures
 * - ERROR: Encryption failure, IO failure, process crash detection
 */
public final class Logger {

    private Logger() {
        // Utility class, no instantiation
    }

    /**
     * Logs an informational message.
     * 
     * @param message the message to log
     */
    public static synchronized void info(String message) {
        System.out.println("[LOG] " + message);
    }

    /**
     * Logs a warning message.
     * 
     * @param message the message to log
     */
    public static synchronized void warn(String message) {
        System.out.println("[LOG] " + message);
    }

    /**
     * Logs an error message.
     * 
     * @param message the message to log
     */
    public static synchronized void error(String message) {
        System.err.println("[LOG] " + message);
    }

    /**
     * Logs an informational message with context.
     * 
     * @param context the context (e.g., class name)
     * @param message the message to log
     */
    public static synchronized void info(String context, String message) {
        System.out.println("[LOG][" + context + "] " + message);
    }

    /**
     * Logs a warning message with context.
     * 
     * @param context the context (e.g., class name)
     * @param message the message to log
     */
    public static synchronized void warn(String context, String message) {
        System.out.println("[LOG][" + context + "] " + message);
    }

    /**
     * Logs an error message with context.
     * 
     * @param context the context (e.g., class name)
     * @param message the message to log
     */
    public static synchronized void error(String context, String message) {
        System.err.println("[LOG][" + context + "] " + message);
    }
}
