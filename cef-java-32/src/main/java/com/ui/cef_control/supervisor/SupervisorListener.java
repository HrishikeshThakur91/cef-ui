package com.ui.cef_control.supervisor;

/**
 * Listener for UI supervisor events.
 * Implementations receive lifecycle notifications from {@link UISupervisor} and should
 * avoid throwing exceptions; the supervisor ignores listener exceptions.
 */
public interface SupervisorListener
{

	/**
	 * Called when the UI becomes available.
	 */
	void onUiAvailable();

	/**
	 * Called when the UI is unavailable (failed to start).
	 */
	void onUiUnavailable();

	/**
	 * Called when the UI crashes.
	 * @param error the error that caused the crash
	 */
	void onUiCrashed(Throwable error);
}
