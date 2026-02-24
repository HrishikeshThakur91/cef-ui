package com.anca.appl.fw.gui.comm_control.config;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Application configuration parsed from command-line arguments.
 */
public final class AppConfig
{

	private final int ipcPort;
	private final String sessionToken;
	private final String startUrl;
	private final String windowId;

	private static final int MIN_PORT = 1024;
	private static final int MAX_PORT = 65535;
	private static final Set<String> REQUIRED_FLAGS = Set.of(
			"--ipcPort",
			"--sessionToken",
			"--startUrl",
			"--windowId"
	);

	/**
	 * Set of all known flags for validation.
	 */
	private static final Set<String> KNOWN_FLAGS = Set.of(
			"--ipcPort",
			"--sessionToken",
			"--startUrl",
			"--windowId"
	);

	/**
	 * Mapping of command-line flags to parameter names for error messages.
	 */
	private static final Map<String, String> FLAG_TO_PARAM = Map.of(
			"--ipcPort", "ipcPort",
			"--sessionToken", "sessionToken",
			"--startUrl", "startUrl",
			"--windowId", "windowId"
	);


	/**
	 * Private constructor to enforce use of fromArgs factory method.
	 *
	 * @param ipcPort - IPC port number
	 * @param sessionToken - Session token for authentication
	 * @param startUrl - URL to load on startup
	 * @param windowId - Unique identifier for the application window
	 */
	private AppConfig(int ipcPort, String sessionToken, String startUrl, String windowId)
	{
		this.ipcPort = ipcPort;
		this.sessionToken = sessionToken;
		this.startUrl = startUrl;
		this.windowId = windowId;
	}


	/**
	 * Parses command-line arguments and constructs an AppConfig instance.
	 *
	 * @param args - command-line arguments
	 *
	 * @return AppConfig instance with parsed values
	 *
	 * @throws InvalidConfigException if parsing or validation fails
	 */
	public static AppConfig fromArgs(String[] args)
	{
		Map<String, String> parsed = parseArgs(args);
		validateAllRequiredPresent(parsed);

		int ipcPort = parseIpcPort(parsed.get("--ipcPort"));
		String sessionToken = validateNonEmpty(parsed.get("--sessionToken"), "sessionToken");
		String startUrl = validateNonEmpty(parsed.get("--startUrl"), "startUrl");
		String windowId = validateNonEmpty(parsed.get("--windowId"), "windowId");

		return new AppConfig(ipcPort, sessionToken, startUrl, windowId);
	}


	/**
	 * Parses command-line arguments into a map of flag to value.
	 * Validates format and known flags.
	 *
	 * @param args - command-line arguments
	 *
	 * @return map of flag to value
	 *
	 * @throws InvalidConfigException if format is invalid or unknown flags are present
	 */
	private static Map<String, String> parseArgs(String[] args)
	{
		Map<String, String> parsed = new HashMap<>();

		for (int i = 0; i < args.length; i++)
		{
			String arg = args[i];

			if (!arg.startsWith("--"))
			{
				throw new InvalidConfigException("Invalid argument format: " + arg);
			}

			if (i + 1 >= args.length || args[i + 1].startsWith("--"))
			{
				throw new InvalidConfigException(
						"Flag " + arg + " requires a value"
				);
			}

			if (!KNOWN_FLAGS.contains(arg))
			{
				throw new InvalidConfigException("Unknown flag: " + arg);
			}

			parsed.put(arg, args[i + 1]);
			i++;
		}

		return parsed;
	}


	/**
	 * Validates that all required flags are present in the parsed arguments.
	 *
	 * @param parsed - map of parsed flags to values
	 */
	private static void validateAllRequiredPresent(Map<String, String> parsed)
	{
		for (String flag : REQUIRED_FLAGS)
		{
			if (!parsed.containsKey(flag))
			{
				String paramName = FLAG_TO_PARAM.get(flag);
				throw new InvalidConfigException(
						"Required parameter missing: " + paramName
				);
			}
		}
	}


	/**
	 * Parses and validates the ipcPort value.
	 *
	 * @param value - string value of ipcPort
	 *
	 * @return integer port number
	 *
	 * @throws InvalidConfigException if parsing fails or port is out of range
	 */
	private static int parseIpcPort(String value)
	{
		int port;
		try
		{
			port = Integer.parseInt(value);
		}
		catch (NumberFormatException e)
		{
			throw new InvalidConfigException(
					"ipcPort must be a valid integer",
					e
			);
		}

		if (port < MIN_PORT || port > MAX_PORT)
		{
			throw new InvalidConfigException(
					"ipcPort must be in range " + MIN_PORT + "-" + MAX_PORT +
							", got: " + port
			);
		}

		return port;
	}


	/**
	 * Validates that a string value is not null, empty, or blank.
	 *
	 * @param value - the string value to validate
	 * @param paramName - the name of the parameter for error messages
	 *
	 * @return the original value if valid
	 *
	 * @throws InvalidConfigException if the value is null, empty, or blank
	 */
	private static String validateNonEmpty(String value, String paramName)
	{
		if (value == null || value.trim().isEmpty())
		{
			throw new InvalidConfigException(
					paramName + " cannot be empty or blank"
			);
		}
		return value;
	}


	public int getIpcPort()
	{
		return ipcPort;
	}


	public String getSessionToken()
	{
		return sessionToken;
	}


	public String getStartUrl()
	{
		return startUrl;
	}


	public String getWindowId()
	{
		return windowId;
	}
}
