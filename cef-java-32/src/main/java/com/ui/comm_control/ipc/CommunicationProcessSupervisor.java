package com.anca.appl.fw.gui.comm_control.ipc;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.LogManager;
import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Communication process lifecycle supervisor.
 * 
 * Manages the Communication process lifecycle including:
 * - Launching Communication with command-line arguments
 * - Monitoring process health
 * - Detecting abnormal termination
 * - Exposing process status
 * 
 * Does NOT auto-restart on crash - caller must decide restart policy.
 * 
 * Thread Safety:
 * - Monitor thread is created on start() and terminates when process exits
 * - Status reads/writes are synchronized via synchronized methods
 * - Happens-before relationship: process exit -> status update -> getStatus()
 * read
 * 
 * Constraints:
 * - No IPC (just process launch)
 * - No encryption (passes key as argument)
 * - No UI logic
 * - No threads beyond Process API and monitor thread
 * - No retries beyond basic restart
 * - Uses Logger for logging
 * - Deterministic behavior
 */
public final class CommunicationProcessSupervisor
{

    /**
     * Process status enumeration.
     */
    public enum ProcessStatus
    {
        /**
         * Process has never been started
         */
        NOT_STARTED,
        /**
         * Process is currently running
         */
        RUNNING,
        /**
         * Process was stopped normally via stop()
         */
        STOPPED,
        /**
         * Process exited unexpectedly
         */
        CRASHED
    }

    private final String cefExecutablePath;
    private final String startUrl;
    private final Path controlFile;
    private final byte[] controlKey;

    private Process cefProcess;
    private volatile ProcessStatus status;
    private Integer exitCode;
    private Thread monitorThread;
    private static Logger logger = LogManager.getLogger();


    /**
     * Creates a new CommunicationProcessSupervisor.
     *
     * @param cefExecutablePath path to the CEF executable
     * @param startUrl initial URL to load
     * @param controlFile path to the control file
     * @param controlKey Base64-encoded encryption key
     *
     * @throws NullPointerException if any parameter is null
     */
    public CommunicationProcessSupervisor(String cefExecutablePath, String startUrl, Path controlFile, byte[] controlKey)
    {
        if (cefExecutablePath == null)
        {
            throw new NullPointerException("cefExecutablePath cannot be null");
        }
        if (startUrl == null)
        {
            throw new NullPointerException("startUrl cannot be null");
        }
        if (controlFile == null)
        {
            throw new NullPointerException("controlFile cannot be null");
        }
        if (controlKey == null)
        {
            throw new NullPointerException("controlKey cannot be null");
        }

        this.cefExecutablePath = cefExecutablePath;
        this.startUrl = startUrl;
        this.controlFile = controlFile;
        this.controlKey = controlKey;
        this.status = ProcessStatus.NOT_STARTED;
        this.cefProcess = null;
        this.exitCode = null;
        this.monitorThread = null;

        logger.info("[CommunicationProcessSupervisor] : " + "Initialized with executable: " + cefExecutablePath);
    }


    /**
     * Starts the Communication process.
     * <p>
     * Launches the CEF executable with command-line arguments:
     * --startUrl, --controlFile, --controlKey
     *
     * @throws IllegalStateException if process is already running
     * @throws IOException if process launch fails
     */
    public synchronized void start() throws IOException
    {
        if (isAlive())
        {
            throw new IllegalStateException("CEF process is already running");
        }

        logger.info("CommunicationProcessSupervisor : Starting CEF process...");

        // Build command line
        List<String> command = new ArrayList<>();
        command.add(cefExecutablePath);
        command.add("--startUrl");
        command.add(startUrl);
        command.add("--controlFile");
        command.add(controlFile.toAbsolutePath().toString());
        command.add("--controlKey");
        command.add(Arrays.toString(controlKey));

        logger.info("CommunicationProcessSupervisor : Command: " + String.join(" ", command));

        // Launch process
        ProcessBuilder processBuilder = new ProcessBuilder(command);
        processBuilder.inheritIO(); // Inherit stdin/stdout/stderr for debugging

//        try
        {
//            cefProcess = processBuilder.start();
//            status = ProcessStatus.RUNNING;
//            exitCode = null;
//
//            // Start monitoring thread
//            startMonitorThread();

            logger.info("CommunicationProcessSupervisor : Communication process started successfully");

        }
//        catch (IOException e)
//        {
//            String errorMsg = "Failed to start Communication process: " + e.getMessage();
//            logger.error("CommunicationProcessSupervisor : " + errorMsg);
//            cefProcess = null;
//            status = ProcessStatus.NOT_STARTED;
//            throw new IOException(errorMsg, e);
//        }
    }


    /**
     * Stops the Communication process gracefully.
     * <p>
     * If the process is not running, this is a no-op.
     * Waits up to 5 seconds for graceful termination.
     */
    public synchronized void stop()
    {
        if (!isAlive())
        {
            logger.info("CommunicationProcessSupervisor : Process is not running, nothing to stop");
            return;
        }

        logger.info("CommunicationProcessSupervisor : Stopping Communication process...");

        // Request graceful shutdown
        cefProcess.destroy();

        // Wait for termination with timeout
        try
        {
            boolean terminated = cefProcess.waitFor(5, java.util.concurrent.TimeUnit.SECONDS);
            if (!terminated)
            {
                logger.warn("CommunicationProcessSupervisor : Process did not terminate gracefully, forcing...");
                cefProcess.destroyForcibly();
                cefProcess.waitFor(2, java.util.concurrent.TimeUnit.SECONDS);
            }

            exitCode = cefProcess.exitValue();
            status = ProcessStatus.STOPPED;

            logger.info("[CommunicationProcessSupervisor] : Communication process stopped (exit code: " + exitCode + ")");

        }
        catch (InterruptedException e)
        {
            Thread.currentThread().interrupt();
            logger.error("[CommunicationProcessSupervisor] : Stop interrupted: " + e.getMessage());
        }
    }


    /**
     * Checks if the Communication process is currently alive.
     *
     * @return true if the process is running, false otherwise
     */
    public boolean isAlive()
    {
        return cefProcess != null && cefProcess.isAlive();
    }


    /**
     * Gets the current process status.
     * Thread-safe: synchronized to ensure visibility of status updates from monitor
     * thread.
     *
     * @return current status
     */
    public synchronized ProcessStatus getStatus()
    {
        return status;
    }


    /**
     * Gets the exit code from the last process termination.
     * Thread-safe: synchronized to ensure visibility of exit code from monitor
     * thread.
     *
     * @return exit code, or null if process has never terminated
     */
    public synchronized Integer getExitCode()
    {
        return exitCode;
    }


    /**
     * Starts the background thread that monitors process exit.
     * <p>
     * Monitor thread lifecycle:
     * - Created when process starts
     * - Blocks on process.waitFor()
     * - Terminates automatically when process exits
     * - Daemon thread (won't prevent JVM shutdown)
     */
    private void startMonitorThread()
    {
        monitorThread = new Thread(() -> {
            try
            {
                int code = cefProcess.waitFor();
                handleProcessExit(code);
            }
            catch (InterruptedException e)
            {
                Thread.currentThread().interrupt();
                logger.info("CommunicationProcessSupervisor : Monitor thread interrupted");
            }
        });

        monitorThread.setDaemon(true);
        monitorThread.setName("Communication-Process-Monitor");
        monitorThread.start();
    }


    /**
     * Handles process exit, updating status and logging.
     *
     * @param code the exit code
     */
    private synchronized void handleProcessExit(int code)
    {
        exitCode = code;

        // Determine if this was a crash or normal exit
        if (status == ProcessStatus.RUNNING)
        {
            if (code != 0)
            {
                status = ProcessStatus.CRASHED;
                logger.error("[CommunicationProcessSupervisor] : Communication process CRASHED with exit code: " + code);
            }
            else
            {
                status = ProcessStatus.STOPPED;
                logger.info("[CommunicationProcessSupervisor] : Communication process exited normally with code: " + code);
            }
        }
        else
        {
            // Already marked as STOPPED by stop() method
            logger.info("[CommunicationProcessSupervisor] : Communication process terminated with code: " + code);
        }
    }
}
