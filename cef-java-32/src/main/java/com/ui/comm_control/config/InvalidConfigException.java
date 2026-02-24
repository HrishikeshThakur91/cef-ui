package com.anca.appl.fw.gui.comm_control.config;

/**
 * Exception thrown when the configuration for the communication controller is invalid.
 * This can occur when required parameters are missing, have invalid formats, or contain
 * values that are out of acceptable ranges.

 * Examples of invalid configurations include:
 * - Missing CEF executable path
 * - Invalid control file path (e.g., non-existent directory)
 * - Control key that is not properly Base64-encoded or not 32 bytes when decoded

 * This exception should be thrown during initialization of the CommunicationController
 * if the provided configuration does not meet the necessary criteria for successful operation.
 */
public class InvalidConfigException extends RuntimeException
{

	/**
	 * Constructs a new InvalidConfigException with the specified detail message.
	 * @param message the detail message explaining the reason for the exception
	 */
	public InvalidConfigException(String message)
	{
		super(message);
	}


	/**
	 * Constructs a new InvalidConfigException with the specified detail message and cause.
	 * @param message the detail message explaining the reason for the exception
	 * @param cause the underlying cause of the exception (e.g., IOException when reading config)
	 */
	public InvalidConfigException(String message, Throwable cause)
	{
		super(message, cause);
	}
}
