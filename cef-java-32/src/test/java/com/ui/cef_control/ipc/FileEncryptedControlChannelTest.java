package com.ui.cef_control.ipc;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Test for FileEncryptedControlChannel.
 * 
 * Verifies:
 * - AES-256-GCM encryption
 * - Atomic file writes
 * - JSON serialization
 * - Error handling
 */
class FileEncryptedControlChannelTest {

    @TempDir
    Path tempDir;

    private Path controlFile;
    private String base64Key;
    private FileEncryptedControlChannel channel;

    @BeforeEach
    void setUp() {
        // Generate a random 32-byte key for AES-256
        byte[] keyBytes = new byte[32];
        new java.security.SecureRandom().nextBytes(keyBytes);
        base64Key = Base64.getEncoder().encodeToString(keyBytes);

        // Create control file path
        controlFile = tempDir.resolve("control.dat");
    }

    @AfterEach
    void tearDown() {
        if (channel != null) {
            channel.shutdown();
        }
    }

    @Test
    void testConstructorValidatesNullControlFile() {
        assertThrows(NullPointerException.class, () -> {
            new FileEncryptedControlChannel(null, base64Key);
        });
    }

    @Test
    void testConstructorValidatesNullKey() {
        assertThrows(NullPointerException.class, () -> {
            new FileEncryptedControlChannel(controlFile, null);
        });
    }

    @Test
    void testConstructorValidatesInvalidBase64Key() {
        assertThrows(IllegalArgumentException.class, () -> {
            new FileEncryptedControlChannel(controlFile, "invalid-base64!");
        });
    }

    @Test
    void testConstructorValidatesKeyLength() {
        // Create a key that's not 32 bytes
        byte[] shortKey = new byte[16];
        String shortBase64Key = Base64.getEncoder().encodeToString(shortKey);

        assertThrows(IllegalArgumentException.class, () -> {
            new FileEncryptedControlChannel(controlFile, shortBase64Key);
        });
    }

    @Test
    void testSendCommandCreatesEncryptedFile() throws Exception {
        channel = new FileEncryptedControlChannel(controlFile, base64Key);

        Map<String, String> payload = new HashMap<>();
        payload.put("url", "https://example.com");

        ControlCommand command = new ControlCommand(
                "cmd-123",
                ControlCommandType.NAVIGATE,
                payload,
                System.currentTimeMillis());

        channel.sendCommand(command);

        // Verify file exists
        assertTrue(Files.exists(controlFile), "Control file should exist");

        // Verify file has content
        byte[] fileContent = Files.readAllBytes(controlFile);
        assertTrue(fileContent.length > 0, "File should have content");

        // Verify file format: at least 12 (IV) + 16 (tag) = 28 bytes
        assertTrue(fileContent.length >= 28, "File should have at least IV + tag");
    }

    @Test
    void testEncryptedFileFormat() throws Exception {
        channel = new FileEncryptedControlChannel(controlFile, base64Key);

        ControlCommand command = new ControlCommand(
                "cmd-456",
                ControlCommandType.START,
                null,
                System.currentTimeMillis());

        channel.sendCommand(command);

        byte[] fileContent = Files.readAllBytes(controlFile);

        // Extract IV (first 12 bytes)
        byte[] iv = new byte[12];
        System.arraycopy(fileContent, 0, iv, 0, 12);

        // Extract ciphertext + tag (remaining bytes)
        byte[] ciphertextWithTag = new byte[fileContent.length - 12];
        System.arraycopy(fileContent, 12, ciphertextWithTag, 0, ciphertextWithTag.length);

        // Decrypt to verify format
        byte[] keyBytes = Base64.getDecoder().decode(base64Key);
        SecretKey secretKey = new SecretKeySpec(keyBytes, "AES");

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcmSpec = new GCMParameterSpec(128, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmSpec);

        byte[] decrypted = cipher.doFinal(ciphertextWithTag);
        String jsonString = new String(decrypted, "UTF-8");

        // Verify JSON contains expected fields
        assertTrue(jsonString.contains("cmd-456"), "JSON should contain command ID");
        assertTrue(jsonString.contains("START"), "JSON should contain command type");
        assertTrue(jsonString.contains("timestamp"), "JSON should contain timestamp");
    }

    @Test
    void testAtomicFileReplacement() throws Exception {
        channel = new FileEncryptedControlChannel(controlFile, base64Key);

        // Send first command
        ControlCommand command1 = new ControlCommand(
                "cmd-1",
                ControlCommandType.NAVIGATE,
                null,
                System.currentTimeMillis());
        channel.sendCommand(command1);

        byte[] firstContent = Files.readAllBytes(controlFile);

        // Send second command (should replace atomically)
        ControlCommand command2 = new ControlCommand(
                "cmd-2",
                ControlCommandType.SHUTDOWN,
                null,
                System.currentTimeMillis());
        channel.sendCommand(command2);

        byte[] secondContent = Files.readAllBytes(controlFile);

        // Verify content changed
        assertNotEquals(firstContent.length, secondContent.length, "File content should change");

        // Verify temp file doesn't exist
        Path tempFile = controlFile.resolveSibling(controlFile.getFileName() + ".tmp");
        assertFalse(Files.exists(tempFile), "Temp file should not exist after atomic move");
    }

    @Test
    void testSendCommandValidatesNullCommand() {
        channel = new FileEncryptedControlChannel(controlFile, base64Key);

        assertThrows(NullPointerException.class, () -> {
            channel.sendCommand(null);
        });
    }

    @Test
    void testSendCommandAfterShutdown() {
        channel = new FileEncryptedControlChannel(controlFile, base64Key);
        channel.shutdown();

        ControlCommand command = new ControlCommand(
                "cmd-999",
                ControlCommandType.HEALTH_PING,
                null,
                System.currentTimeMillis());

        assertThrows(RuntimeException.class, () -> {
            channel.sendCommand(command);
        });
    }

    @Test
    void testPayloadSerialization() throws Exception {
        channel = new FileEncryptedControlChannel(controlFile, base64Key);

        Map<String, String> payload = new HashMap<>();
        payload.put("url", "https://test.com");
        payload.put("title", "Test Page");

        ControlCommand command = new ControlCommand(
                "cmd-payload",
                ControlCommandType.NAVIGATE,
                payload,
                12345L);

        channel.sendCommand(command);

        // Decrypt and verify payload
        byte[] fileContent = Files.readAllBytes(controlFile);
        byte[] iv = new byte[12];
        System.arraycopy(fileContent, 0, iv, 0, 12);

        byte[] ciphertextWithTag = new byte[fileContent.length - 12];
        System.arraycopy(fileContent, 12, ciphertextWithTag, 0, ciphertextWithTag.length);

        byte[] keyBytes = Base64.getDecoder().decode(base64Key);
        SecretKey secretKey = new SecretKeySpec(keyBytes, "AES");

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcmSpec = new GCMParameterSpec(128, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmSpec);

        byte[] decrypted = cipher.doFinal(ciphertextWithTag);
        String jsonString = new String(decrypted, "UTF-8");

        assertTrue(jsonString.contains("https://test.com"), "JSON should contain URL");
        assertTrue(jsonString.contains("Test Page"), "JSON should contain title");
    }
}
