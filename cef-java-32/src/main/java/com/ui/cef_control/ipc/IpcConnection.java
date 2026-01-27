package com.ui.cef_control.ipc;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class IpcConnection {
    private final Socket socket;
    private final BufferedReader reader;
    private final BufferedWriter writer;
    private final List<IpcConnectionListener> listeners;
    private volatile boolean running;
    private Thread readerThread;

    public interface IpcConnectionListener {
        void onMessageReceived(IpcMessage message);
        void onConnectionClosed();
        void onError(Throwable error);
    }

    public IpcConnection(Socket socket) throws IOException {
        this.socket = socket;
        this.reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        this.writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
        this.listeners = new ArrayList<>();
        this.running = true;
    }

    public synchronized void start() {
        readerThread = new Thread(this::readLoop, "IpcConnection-Reader");
        readerThread.setDaemon(false);
        readerThread.start();
    }

    public synchronized void close() throws IOException {
        running = false;
        if (readerThread != null && readerThread.isAlive()) {
            readerThread.interrupt();
        }
        try {
            reader.close();
            writer.close();
            socket.close();
        } catch (IOException e) {
            System.err.println("Error closing IPC connection: " + e);
        }
        notifyConnectionClosed();
    }

    public synchronized void sendMessage(IpcMessage message) throws IOException {
        if (!running) {
            throw new IOException("Connection is closed");
        }
        String json = message.toJson();
        writer.write(json);
        writer.newLine();
        writer.flush();
    }

    public void addListener(IpcConnectionListener listener) {
        if (listener != null && !listeners.contains(listener)) {
            listeners.add(listener);
        }
    }

    public void removeListener(IpcConnectionListener listener) {
        listeners.remove(listener);
    }

    public boolean isConnected() {
        return running && socket.isConnected() && !socket.isClosed();
    }

    private void readLoop() {
        try {
            String line;
            while (running && (line = reader.readLine()) != null) {
                try {
                    IpcMessage message = IpcMessage.fromJson(line);
                    notifyMessageReceived(message);
                } catch (Exception e) {
                    notifyError(e);
                }
            }
        } catch (IOException e) {
            if (running) {
                notifyError(e);
            }
        } finally {
            running = false;
            notifyConnectionClosed();
        }
    }

    private void notifyMessageReceived(IpcMessage message) {
        List<IpcConnectionListener> snapshot = new ArrayList<>(listeners);
        for (IpcConnectionListener listener : snapshot) {
            try {
                listener.onMessageReceived(message);
            } catch (Exception e) {
                System.err.println("Listener error: " + e);
            }
        }
    }

    private void notifyConnectionClosed() {
        List<IpcConnectionListener> snapshot = new ArrayList<>(listeners);
        for (IpcConnectionListener listener : snapshot) {
            try {
                listener.onConnectionClosed();
            } catch (Exception e) {
                System.err.println("Listener error: " + e);
            }
        }
    }

    private void notifyError(Throwable error) {
        List<IpcConnectionListener> snapshot = new ArrayList<>(listeners);
        for (IpcConnectionListener listener : snapshot) {
            try {
                listener.onError(error);
            } catch (Exception e) {
                System.err.println("Listener error: " + e);
            }
        }
    }
}

