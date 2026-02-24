package com.anca.appl.fw.gui.comm_control.ipc;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.LogManager;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * Main controller for CEF browser lifecycle.
 * Provides simple API for starting/stopping CEF and sending commands.
 * Thread-safe for basic operations.
 * Usage:
 * CommunicationController controller = new CommunicationController(cefExePath, controlFile, controlKey);
 * controller.startCommunication(startUrl);
 * controller.navigateTo(url);
 * controller.stopCommunication();
 */
public class CommunicationController
{

	private final String cefExecutablePath;
	private final Path controlFile;
	private final byte[] controlKey;

	private CommunicationProcessSupervisor processSupervisor;
	private FileEncryptedControlChannel controlChannel;
	private volatile boolean initialized = false;
	private static Logger logger = LogManager.getLogger();


	/**
	 * Creates a new Communication controller.
	 * @param cefExecutablePath path to CEF executable
	 * @param controlFilePath path to control file for commands
	 * @param controlKey Base64-encoded AES-256 key
	 */
	public CommunicationController(String cefExecutablePath, String controlFilePath, byte[] controlKey)
	{
		this.cefExecutablePath = cefExecutablePath;
		this.controlFile = Paths.get(controlFilePath);
		this.controlKey = controlKey;
		this.initialized = true;
		// logger.info("CommunicationController", "Controller created with executable: " + cefExecutablePath);
	}


	/**
	 * Starts the Communication process if not already running.
	 * Workflow:
	 * 1. Check if CEF is already running
	 * 2. If not, initialize control channel
	 * 3. Start CEF process with control file/key arguments
	 * 4. Send START command
	 * 5. Send NAVIGATE command with URL
	 * @param startUrl the URL to load on startup
	 * @throws IllegalStateException if already running
	 * @throws IOException if process start fails
	 */
	public synchronized void startCommunication(String startUrl) throws IOException
	{
		logger.info("CommunicationController : startCommunication() called with URL: " + startUrl);

		// // Step 1: Check if already running
		// if (isCommunicationRunning()) {
		// 	logger.warn("CommunicationController : CEF is already running, ignoring start request");
		// 	throw new IllegalStateException("CEF is already running");
		// }

		try
		{
			// Step 2: Initialize control channel
			logger.info("CommunicationController : Initializing control channel...");
			controlChannel = new FileEncryptedControlChannel(controlFile, controlKey);

			// Step 3: Initialize and start process supervisor
			logger.info("CommunicationController : Starting Communication process...");
			processSupervisor = new CommunicationProcessSupervisor(
					cefExecutablePath,
					startUrl,
					controlFile,
					controlKey);
			processSupervisor.start();

			// Wait a bit for process to initialize
			Thread.sleep(1000);

			// Step 4: Send START command
			logger.info("CommunicationController :Sending START command...");
			Map<String, String> payload = new HashMap<>();
			payload.put("url", startUrl);
			sendCommand(ControlCommandType.START, payload);

			initialized = true;
			logger.info("CommunicationController :Communication started successfully");

		}
		catch (Exception e) {
			cleanup();

			String logMsg;
			String exceptionMsg;

			if (e instanceof InterruptedException) {
				// Special handling for InterruptedException
				Thread.currentThread().interrupt(); 
				logMsg = "Interrupted during startup: ";
				exceptionMsg = "Communication startup interrupted";
			} else {
				// Standard handling for all other exceptions
				logMsg = "Failed to start CEF: ";
				exceptionMsg = "Failed to start CEF";
			}

			// 3. Shared Logging and Rethrowing
			logger.error("[CommunicationController] :" + logMsg + e.getMessage());
			throw new IOException(exceptionMsg, e);
		}
	}


	/**
	 * Navigates to a new URL.
	 * Sends a NAVIGATE command to the CEF process via the encrypted control channel.
	 * The command is written to the control.dat file as encrypted JSON.
	 * Workflow:
	 * 1. Validate URL is not null/empty
	 * 2. Check if control channel is initialized
	 * 3. Create NAVIGATE command with URL payload
	 * 4. Serialize to JSON and encrypt with AES-256-GCM
	 * 5. Write atomically to control.dat file
	 * 6. CEF process reads and executes the command
	 * @param url the URL to navigate to
	 * @throws IllegalStateException if control channel is not initialized
	 * @throws NullPointerException if url is null
	 */
	public void navigateTo(String url) throws IOException, InterruptedException
	{
		logger.info("CommunicationController : navigateTo() called with URL: " + url);

		// Step 1: Validate URL
		if (url == null || url.trim().isEmpty())
		{
			logger.error("CommunicationController :Cannot navigate: CEF is not running");
			throw new IllegalStateException("CEF is not running");
		}

		// Step 2: Check control channel is ready
		if (controlChannel == null)
		{
			String errorMsg = "Control channel not initialized. Call startCef() first.";
			//			Logger.error("CefController" + errorMsg);
			//			throw new IllegalStateException(errorMsg);
			controlChannel = new FileEncryptedControlChannel(controlFile, controlKey);
			processSupervisor = new CommunicationProcessSupervisor(
					cefExecutablePath,
					url,
					controlFile,
					controlKey);
			processSupervisor.start();
			// Wait a bit for process to initialize
			Thread.sleep(1000);

//			// Step 4: Send START command
//			logger.info("CefController : Sending START command...");
//			sendCommand(ControlCommandType.START, null);

			// Step 5: Send NAVIGATE command
			logger.info("CefController : Sending NAVIGATE command...");
			Map<String, String> payload = new HashMap<>();
			payload.put("url", url);
			sendCommand(ControlCommandType.NAVIGATE, payload);

		}
		else
		{
			try
			{
				// Step 3-5: Create and send command via encrypted control file
				logger.info("CefController", "Creating NAVIGATE command with URL: " + url);
				Map<String, String> payload = new HashMap<>();
				payload.put("url", url);
				sendCommand(ControlCommandType.NAVIGATE, payload);

				logger.info("CefController : Navigate command sent to control.dat");

			}
			catch (Exception e)
			{
				String errorMsg = "Failed to send navigate command: " + e.getMessage();
				logger.error("CefController : " + errorMsg);
				throw new RuntimeException(errorMsg, e);
			}
		}
	}


	/**
	 * Stops the CEF process gracefully.
	 */
	public synchronized void stopCommunication()
	{
		logger.info("CommunicationController : stopCommunication() called");

		if (!isCommunicationRunning())
		{
			logger.warn("CommunicationController : CEF is not running, nothing to stop");
			return;
		}

		try
		{
			// Send shutdown command
			logger.info("CommunicationController : Sending SHUTDOWN command...");
			sendCommand(ControlCommandType.SHUTDOWN, null);

			// Wait a bit for graceful shutdown
			Thread.sleep(500);

			// Stop process supervisor
			if (processSupervisor != null)
			{
				processSupervisor.stop();
			}

			logger.info("CommunicationController : CEF stopped successfully");

		}
		catch (InterruptedException e)
		{
			Thread.currentThread().interrupt();
			logger.error("CommunicationController : Interrupted during shutdown: " + e.getMessage());
		}
		finally
		{
			cleanup();
		}
	}


	/**
	 * Checks if CEF process is currently running.
	 * @return true if running, false otherwise
	 */
	public boolean isCommunicationRunning()
	{
		return processSupervisor != null && processSupervisor.isAlive();
	}


	/**
	 * Gets the current process status.
	 * @return the process status, or NOT_STARTED if not initialized
	 */
	public CommunicationProcessSupervisor.ProcessStatus getStatus()
	{
		if (processSupervisor == null)
		{
			return CommunicationProcessSupervisor.ProcessStatus.NOT_STARTED;
		}
		return processSupervisor.getStatus();
	}


	/**
	 * Sends a command to the CEF process.
	 *
	 * @param type the command type
	 * @param payload optional payload (can be null)
	 */
	private void sendCommand(ControlCommandType type, Map<String, String> payload)
	{
		if (controlChannel == null)
		{
			logger.error("CommunicationController : Control channel not initialized");
			throw new IllegalStateException("Control channel not initialized");
		}

		String commandId = UUID.randomUUID().toString();
		ControlCommand command = new ControlCommand(commandId, type, payload);

		try
		{
			controlChannel.sendCommand(command);
		}
		catch (Exception e)
		{
			logger.error("CommunicationController : Failed to send command: " + e.getMessage());
			throw new RuntimeException("Failed to send command", e);
		}
	}


	/**
	 * Cleans up resources.
	 */
	private void cleanup()
	{
		if (controlChannel != null)
		{
			controlChannel.shutdown();
			controlChannel = null;
		}
		processSupervisor = null;
		initialized = false;
	}
}