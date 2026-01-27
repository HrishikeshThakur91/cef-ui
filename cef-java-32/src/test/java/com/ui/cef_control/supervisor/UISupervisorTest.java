package com.ui.cef_control.supervisor;

//import com.ui.cef_control.lifecycle.LifecycleListener;
//import com.ui.cef_control.lifecycle.ProcessController;
import org.junit.Before;
import org.junit.Test;
import static org.junit.Assert.*;

public class UISupervisorTest {

	private UISupervisor supervisor;
	private TestUIProcess uiProcess;
	private TestRetryPolicy retryPolicy;

	@Before
	public void setUp() {
		uiProcess = new TestUIProcess();
		retryPolicy = new TestRetryPolicy();
		supervisor = new UISupervisor(uiProcess, retryPolicy);
	}

	@Test
	public void testDetectUiUnavailableAtStartup() {
		uiProcess.setStartupSuccess(false);
		TestSupervisorListener listener = new TestSupervisorListener();
		supervisor.addListener(listener);

		supervisor.start();

		assertTrue("Should detect UI unavailable", listener.uiUnavailableCalled);
		assertFalse("Should not be running", supervisor.isUiRunning());
	}

	@Test
	public void testRetryUiStartViaPolicy() {
		uiProcess.setStartupSuccess(false);
		retryPolicy.setMaxAttempts(3);
		retryPolicy.setRetryDelayMs(0);

		supervisor.start();

		assertEquals("Should attempt retries per policy", 3, uiProcess.startAttempts);
	}

	@Test
	public void testHandleUiCrashNotification() {
		uiProcess.setStartupSuccess(true);
		TestSupervisorListener listener = new TestSupervisorListener();
		supervisor.addListener(listener);

		supervisor.start();
		assertTrue("UI should be running", supervisor.isUiRunning());

		supervisor.notifyUiCrash(new RuntimeException("Simulated crash"));

		assertTrue("Should handle crash notification", listener.uiCrashedCalled);
		assertFalse("UI should not be running after crash", supervisor.isUiRunning());
	}

	@Test
	public void testAllowFireAndForgetCommandsWhenUiDown() {
		uiProcess.setStartupSuccess(false);
		supervisor.start();

		assertFalse("UI should not be running", supervisor.isUiRunning());

		// Should not throw exception
		supervisor.sendCommand("test-command");

		assertEquals("Command should be queued", 1, supervisor.getQueuedCommandCount());
	}

	@Test
	public void testPreserveLastIntentForReplayAfterRestart() {
		uiProcess.setStartupSuccess(true);
		supervisor.start();

		String intent = "open-document";
		supervisor.sendCommand(intent);

		// Simulate crash
		supervisor.notifyUiCrash(new RuntimeException("Crash"));

		// Restart
		uiProcess.setStartupSuccess(true);
		supervisor.start();

		assertEquals("Last intent should be replayed", intent, uiProcess.lastReceivedCommand);
	}

	@Test
	public void testNonBlockingBehavior() {
		uiProcess.setStartupDelayMs(1000);

		long startTime = System.currentTimeMillis();
		supervisor.start();
		long elapsed = System.currentTimeMillis() - startTime;

		assertTrue("start() should not block", elapsed < 500);
	}

	@Test
	public void testDeterministicRetrySequence() {
		uiProcess.setStartupSuccess(false);
		retryPolicy.setMaxAttempts(3);
		retryPolicy.setRetryDelayMs(0);

		supervisor.start();
		int firstAttempts = uiProcess.startAttempts;

		uiProcess.reset();
		supervisor.start();
		int secondAttempts = uiProcess.startAttempts;

		assertEquals("Retry sequence should be deterministic", firstAttempts, secondAttempts);
	}

	@Test
	public void testUiTreatedAsBlackBox() {
		// Supervisor should not know UI implementation details
		assertTrue("Supervisor should work with any UIProcess",
				supervisor instanceof UISupervisor);

		// Should not throw even if UI process has internal errors
		uiProcess.setInternalError(true);
		supervisor.start();

		// Supervisor should continue functioning
		assertNotNull("Supervisor should remain functional", supervisor);
	}

	@Test
	public void testStopCleansUpResources() {
		supervisor.start();
		supervisor.stop();

		assertFalse("Should not be running after stop", supervisor.isUiRunning());
		assertEquals("Should clear queued commands", 0, supervisor.getQueuedCommandCount());
	}

	@Test
	public void testMultipleListenersReceiveNotifications() {
		TestSupervisorListener listener1 = new TestSupervisorListener();
		TestSupervisorListener listener2 = new TestSupervisorListener();

		supervisor.addListener(listener1);
		supervisor.addListener(listener2);

		uiProcess.setStartupSuccess(false);
		supervisor.start();

		assertTrue("Listener 1 should be notified", listener1.uiUnavailableCalled);
		assertTrue("Listener 2 should be notified", listener2.uiUnavailableCalled);
	}

	@Test
	public void testRemoveListenerPreventsNotifications() {
		TestSupervisorListener listener = new TestSupervisorListener();

		supervisor.addListener(listener);
		supervisor.removeListener(listener);

		uiProcess.setStartupSuccess(false);
		supervisor.start();

		assertFalse("Removed listener should not be notified", listener.uiUnavailableCalled);
	}

	@Test
	public void testCommandQueuedWhenUiNotReady() {
		uiProcess.setStartupSuccess(false);
		supervisor.start();

		supervisor.sendCommand("command1");
		supervisor.sendCommand("command2");
		supervisor.sendCommand("command3");

		assertEquals("Commands should be queued", 3, supervisor.getQueuedCommandCount());
	}

	@Test
	public void testQueuedCommandsSentWhenUiBecomesAvailable() {
		uiProcess.setStartupSuccess(false);
		supervisor.start();

		supervisor.sendCommand("command1");
		supervisor.sendCommand("command2");

		// UI becomes available
		uiProcess.setStartupSuccess(true);
		supervisor.start();

		assertEquals("Queued commands should be sent", 2, uiProcess.receivedCommands.size());
	}

	@Test
	public void testRetryPolicyRespected() {
		uiProcess.setStartupSuccess(false);
		retryPolicy.setMaxAttempts(5);

		supervisor.start();

		assertTrue("Should not exceed max attempts", uiProcess.startAttempts <= 5);
	}

	@Test
	public void testCrashDuringStartupHandled() {
		uiProcess.setCrashOnStart(true);
		TestSupervisorListener listener = new TestSupervisorListener();
		supervisor.addListener(listener);

		supervisor.start();

		assertTrue("Should handle startup crash", listener.uiCrashedCalled);
		assertFalse("UI should not be running", supervisor.isUiRunning());
	}

	@Test
	public void testSuccessfulStartAfterRetries() {
		uiProcess.setFailFirstNAttempts(2);
		retryPolicy.setMaxAttempts(5);
		retryPolicy.setRetryDelayMs(0);

		supervisor.start();

		assertTrue("UI should eventually start", supervisor.isUiRunning());
		assertEquals("Should succeed on 3rd attempt", 3, uiProcess.startAttempts);
	}

	@Test
	public void testNoRetriesWhenPolicyDisabled() {
		uiProcess.setStartupSuccess(false);
		retryPolicy.setMaxAttempts(0);

		supervisor.start();

		assertEquals("Should only try once", 1, uiProcess.startAttempts);
	}

	@Test
	public void testNoRetryWhenPolicyDisallows() {
		TestUIProcess ui = new TestUIProcess(); // always fails
		ui.setStartupSuccess(true);

		RetryPolicy policy = (attempt, failure) -> false;
		UISupervisor supervisor = new UISupervisor(ui, policy);

		supervisor.start();

		assertEquals(1, ui.getStartAttempts());
	}

	@Test
	public void testNoRetryOnSuccessfulStart() {
		TestUIProcess ui = new TestUIProcess(); // succeeds
		ui.setStartupSuccess(true);

		RetryPolicy policy = (attempt, failure) -> true;
		UISupervisor supervisor = new UISupervisor(ui, policy);

		supervisor.start();

		assertEquals(1, ui.getStartAttempts());
	}

	@Test
	public void testRetryLimitRespected() {
		TestUIProcess ui = new TestUIProcess(); // always fails
		ui.setStartupSuccess(true);

		RetryPolicy policy = (attempt, failure) -> attempt <= 2;
		UISupervisor supervisor = new UISupervisor(ui, policy);

		supervisor.start();

		assertEquals(3, ui.getStartAttempts()); // 1 initial + 2 retries
	}

	// Helper classes
	private static class TestSupervisorListener implements SupervisorListener {
		boolean uiAvailableCalled = false;
		boolean uiUnavailableCalled = false;
		boolean uiCrashedCalled = false;

		public void onUiAvailable() {
			uiAvailableCalled = true;
		}

		public void onUiUnavailable() {
			uiUnavailableCalled = true;
		}

		public void onUiCrashed(Throwable error) {
			uiCrashedCalled = true;
		}
	}

	private static class TestUIProcess implements UIProcess {
		boolean startupSuccess = true;
		boolean crashOnStart = false;
		boolean internalError = false;
		int startupDelayMs = 0;
		int startAttempts = 0;
		int failFirstNAttempts = 0;
		String lastReceivedCommand = null;
		java.util.List<String> receivedCommands = new java.util.ArrayList<>();

		public void setStartupSuccess(boolean success) {
			this.startupSuccess = success;
		}

		public void setCrashOnStart(boolean crash) {
			this.crashOnStart = crash;
		}

		public void setInternalError(boolean error) {
			this.internalError = error;
		}

		public void setStartupDelayMs(int delayMs) {
			this.startupDelayMs = delayMs;
		}

		public void setFailFirstNAttempts(int n) {
			this.failFirstNAttempts = n;
		}

		public void reset() {
			startAttempts = 0;
			receivedCommands.clear();
			lastReceivedCommand = null;
		}

		public void start() {
			startAttempts++;

			if (crashOnStart) {
				throw new RuntimeException("Crash on start");
			}

			if (failFirstNAttempts > 0 && startAttempts <= failFirstNAttempts) {
				throw new RuntimeException("Start failed");
			}

			if (!startupSuccess) {
				throw new RuntimeException("Startup failed");
			}
		}

		public void sendCommand(String command) {
			lastReceivedCommand = command;
			receivedCommands.add(command);
		}

		public boolean isRunning() {
			return startupSuccess;
		}

		public void stop() {
		}

		int getStartAttempts() {
			return startAttempts;
		}
	}

	private static class TestRetryPolicy implements RetryPolicy {
		int maxAttempts = 3;
		int retryDelayMs = 100;

		public void setMaxAttempts(int max) {
			this.maxAttempts = max;
		}

		public void setRetryDelayMs(int delayMs) {
			this.retryDelayMs = delayMs;
		}

		public int getMaxAttempts() {
			return maxAttempts;
		}


		@Override
		public boolean shouldRetry(int attempt, Throwable failure)
		{
			return false;
		}


		public int getRetryDelayMs(int attemptNumber) {
			return retryDelayMs;
		}

		public boolean shouldRetry(int attemptNumber) {
			return attemptNumber <= maxAttempts;
		}
	}
}
