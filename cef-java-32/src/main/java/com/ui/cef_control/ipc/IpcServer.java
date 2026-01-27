package com.ui.cef_control.ipc;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class IpcServer {
    private final int requestedPort;
    private ServerSocket serverSocket;
    private volatile boolean running;
    private IpcConnection clientConnection;
    private Thread acceptThread;
    private final List<IpcServerListener> listeners;

    public interface IpcServerListener {
        void onClientConnected(IpcConnection connection);
        void onClientDisconnected();
        void onServerError(Throwable error);
    }

    public IpcServer(int requestedPort) {
        this.requestedPort = requestedPort;
        this.listeners = new ArrayList<>();
        this.running = false;
    }

    public synchronized void start() throws IOException {
        if (running) {
            throw new IllegalStateException("Server already running");
        }

        serverSocket = new ServerSocket(requestedPort);
        running = true;

        acceptThread = new Thread(this::acceptLoop, "IpcServer-Accept");
        acceptThread.setDaemon(false);
        acceptThread.start();

        notifyServerStarted();
    }

    public synchronized void stop() throws IOException {
        if (!running) {
            throw new IllegalStateException("Server not running");
        }

        running = false;

        if (clientConnection != null) {
            try {
                clientConnection.close();
            } catch (IOException e) {
                System.err.println("Error closing client connection: " + e);
            }
            clientConnection = null;
        }

        if (serverSocket != null && !serverSocket.isClosed()) {
            serverSocket.close();
        }

        if (acceptThread != null && acceptThread.isAlive()) {
            acceptThread.interrupt();
        }
    }

    public synchronized int getBoundPort() {
        if (serverSocket == null || serverSocket.isClosed()) {
            return -1;
        }
        return serverSocket.getLocalPort();
    }

    public synchronized IpcConnection getClientConnection() {
        return clientConnection;
    }

    public synchronized boolean isRunning() {
        return running;
    }

    public synchronized boolean hasClient() {
        return clientConnection != null && clientConnection.isConnected();
    }

    public void addListener(IpcServerListener listener) {
        if (listener != null && !listeners.contains(listener)) {
            listeners.add(listener);
        }
    }

    public void removeListener(IpcServerListener listener) {
        listeners.remove(listener);
    }

    private void acceptLoop() {
        try {
            while (running && !serverSocket.isClosed()) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    handleNewClient(clientSocket);
                } catch (IOException e) {
                    if (running) {
                        notifyServerError(e);
                    }
                }
            }
        } finally {
            running = false;
        }
    }

    private synchronized void handleNewClient(Socket clientSocket) {
        if (clientConnection != null && clientConnection.isConnected()) {
            try {
                clientSocket.close();
            } catch (IOException e) {
                System.err.println("Error rejecting extra client: " + e);
            }
            return;
        }

        try {
            clientConnection = new IpcConnection(clientSocket);
            clientConnection.addListener(new IpcConnection.IpcConnectionListener() {
                @Override
                public void onMessageReceived(IpcMessage message) {
                    // Message routing deferred
                }

                @Override
                public void onConnectionClosed() {
                    synchronized (IpcServer.this) {
                        clientConnection = null;
                    }
                    notifyClientDisconnected();
                }

                @Override
                public void onError(Throwable error) {
                    notifyServerError(error);
                }
            });
            clientConnection.start();
            notifyClientConnected(clientConnection);
        } catch (IOException e) {
            notifyServerError(e);
        }
    }

    private void notifyServerStarted() {
        // Server start notification
    }

    private void notifyClientConnected(IpcConnection connection) {
        List<IpcServerListener> snapshot = new ArrayList<>(listeners);
        for (IpcServerListener listener : snapshot) {
            try {
                listener.onClientConnected(connection);
            } catch (Exception e) {
                System.err.println("Listener error: " + e);
            }
        }
    }

    private void notifyClientDisconnected() {
        List<IpcServerListener> snapshot = new ArrayList<>(listeners);
        for (IpcServerListener listener : snapshot) {
            try {
                listener.onClientDisconnected();
            } catch (Exception e) {
                System.err.println("Listener error: " + e);
            }
        }
    }

    private void notifyServerError(Throwable error) {
        List<IpcServerListener> snapshot = new ArrayList<>(listeners);
        for (IpcServerListener listener : snapshot) {
            try {
                listener.onServerError(error);
            } catch (Exception e) {
                System.err.println("Listener error: " + e);
            }
        }
    }
}

