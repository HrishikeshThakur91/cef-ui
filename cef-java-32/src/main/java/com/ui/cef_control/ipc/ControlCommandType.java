package com.ui.cef_control.ipc;

/**
 * Enumeration of control command types.
 * 
 * Defines the types of commands that can be sent to the CEF process.
 * This is a pure contract enum with no implementation logic.
 */
public enum ControlCommandType {
    /**
     * Start the CEF browser process.
     */
    START,

    /**
     * Navigate to a specific URL.
     */
    NAVIGATE,

    /**
     * Shutdown the CEF browser process.
     */
    SHUTDOWN,

    /**
     * Health ping to check if the CEF process is responsive.
     */
    HEALTH_PING
}
