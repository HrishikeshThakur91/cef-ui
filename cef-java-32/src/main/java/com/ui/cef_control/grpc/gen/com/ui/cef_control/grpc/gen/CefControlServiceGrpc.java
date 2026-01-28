package com.ui.cef_control.grpc.gen;

import static io.grpc.stub.ClientCalls.asyncUnaryCall;
import static io.grpc.stub.ClientCalls.asyncServerStreamingCall;
import static io.grpc.stub.ClientCalls.asyncClientStreamingCall;
import static io.grpc.stub.ClientCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ClientCalls.blockingUnaryCall;
import static io.grpc.stub.ClientCalls.blockingServerStreamingCall;
import static io.grpc.stub.ClientCalls.futureUnaryCall;
import static io.grpc.MethodDescriptor.generateFullMethodName;
import static io.grpc.stub.ServerCalls.asyncUnaryCall;
import static io.grpc.stub.ServerCalls.asyncServerStreamingCall;
import static io.grpc.stub.ServerCalls.asyncClientStreamingCall;
import static io.grpc.stub.ServerCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedStreamingCall;

/**
 * <pre>
 **
 * Phase-6 gRPC Service
 * Single unary RPC per command type.
 * CEF is client. Java is server.
 * All methods are unary (request ? response).
 * No streaming, no bidirectional communication at gRPC level.
 * Phase-7 TODO: Consider adding streaming for high-frequency events (metrics, logs).
 * </pre>
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.1.2)",
    comments = "Source: cef_service.proto")
public class CefControlServiceGrpc {

  private CefControlServiceGrpc() {}

  public static final String SERVICE_NAME = "cefcontrol.CefControlService";

  // Static method descriptors that strictly reflect the proto.
  @io.grpc.ExperimentalApi("https://github.com/grpc/grpc-java/issues/1901")
  public static final io.grpc.MethodDescriptor<com.ui.cef_control.grpc.gen.HandshakeRequest,
      com.ui.cef_control.grpc.gen.HandshakeResponse> METHOD_HANDSHAKE =
      io.grpc.MethodDescriptor.create(
          io.grpc.MethodDescriptor.MethodType.UNARY,
          generateFullMethodName(
              "cefcontrol.CefControlService", "Handshake"),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.HandshakeRequest.getDefaultInstance()),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.HandshakeResponse.getDefaultInstance()));
  @io.grpc.ExperimentalApi("https://github.com/grpc/grpc-java/issues/1901")
  public static final io.grpc.MethodDescriptor<com.ui.cef_control.grpc.gen.OpenPageRequest,
      com.ui.cef_control.grpc.gen.OpenPageResponse> METHOD_OPEN_PAGE =
      io.grpc.MethodDescriptor.create(
          io.grpc.MethodDescriptor.MethodType.UNARY,
          generateFullMethodName(
              "cefcontrol.CefControlService", "OpenPage"),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.OpenPageRequest.getDefaultInstance()),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.OpenPageResponse.getDefaultInstance()));
  @io.grpc.ExperimentalApi("https://github.com/grpc/grpc-java/issues/1901")
  public static final io.grpc.MethodDescriptor<com.ui.cef_control.grpc.gen.PageStatusRequest,
      com.ui.cef_control.grpc.gen.PageStatusResponse> METHOD_PAGE_STATUS =
      io.grpc.MethodDescriptor.create(
          io.grpc.MethodDescriptor.MethodType.UNARY,
          generateFullMethodName(
              "cefcontrol.CefControlService", "PageStatus"),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.PageStatusRequest.getDefaultInstance()),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.PageStatusResponse.getDefaultInstance()));
  @io.grpc.ExperimentalApi("https://github.com/grpc/grpc-java/issues/1901")
  public static final io.grpc.MethodDescriptor<com.ui.cef_control.grpc.gen.ShutdownRequest,
      com.ui.cef_control.grpc.gen.ShutdownResponse> METHOD_SHUTDOWN =
      io.grpc.MethodDescriptor.create(
          io.grpc.MethodDescriptor.MethodType.UNARY,
          generateFullMethodName(
              "cefcontrol.CefControlService", "Shutdown"),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.ShutdownRequest.getDefaultInstance()),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.ShutdownResponse.getDefaultInstance()));

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static CefControlServiceStub newStub(io.grpc.Channel channel) {
    return new CefControlServiceStub(channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static CefControlServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    return new CefControlServiceBlockingStub(channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary and streaming output calls on the service
   */
  public static CefControlServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    return new CefControlServiceFutureStub(channel);
  }

  /**
   * <pre>
   **
   * Phase-6 gRPC Service
   * Single unary RPC per command type.
   * CEF is client. Java is server.
   * All methods are unary (request ? response).
   * No streaming, no bidirectional communication at gRPC level.
   * Phase-7 TODO: Consider adding streaming for high-frequency events (metrics, logs).
   * </pre>
   */
  public static abstract class CefControlServiceImplBase implements io.grpc.BindableService {

    /**
     * <pre>
     **
     * Handshake RPC.
     * CEF initiates this on connection.
     * Java validates and accepts/rejects the session.
     * This MUST be the first RPC called by CEF.
     * </pre>
     */
    public void handshake(com.ui.cef_control.grpc.gen.HandshakeRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.HandshakeResponse> responseObserver) {
      asyncUnimplementedUnaryCall(METHOD_HANDSHAKE, responseObserver);
    }

    /**
     * <pre>
     **
     * Open Page RPC.
     * Java sends OpenPageRequest.
     * CEF responds immediately with OpenPageResponse (accepted or rejected).
     * CEF then asynchronously sends PageStatusNotification messages
     * (via a separate callback mechanism or polling).
     * Phase-7 TODO: Implement notification delivery (polling or push).
     * </pre>
     */
    public void openPage(com.ui.cef_control.grpc.gen.OpenPageRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.OpenPageResponse> responseObserver) {
      asyncUnimplementedUnaryCall(METHOD_OPEN_PAGE, responseObserver);
    }

    /**
     * <pre>
     **
     * Query Page Status RPC.
     * Java sends PageStatusRequest to CEF.
     * CEF responds immediately with current PageStatusResponse.
     * Used to poll current page load state synchronously.
     * Phase-7 TODO: Add polling mechanism for continuous updates.
     * </pre>
     */
    public void pageStatus(com.ui.cef_control.grpc.gen.PageStatusRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.PageStatusResponse> responseObserver) {
      asyncUnimplementedUnaryCall(METHOD_PAGE_STATUS, responseObserver);
    }

    /**
     * <pre>
     **
     * Shutdown RPC.
     * Java instructs CEF to terminate.
     * Phase-7 TODO: Not required for MVP.
     * </pre>
     */
    public void shutdown(com.ui.cef_control.grpc.gen.ShutdownRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.ShutdownResponse> responseObserver) {
      asyncUnimplementedUnaryCall(METHOD_SHUTDOWN, responseObserver);
    }

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
          .addMethod(
            METHOD_HANDSHAKE,
            asyncUnaryCall(
              new MethodHandlers<
                com.ui.cef_control.grpc.gen.HandshakeRequest,
                com.ui.cef_control.grpc.gen.HandshakeResponse>(
                  this, METHODID_HANDSHAKE)))
          .addMethod(
            METHOD_OPEN_PAGE,
            asyncUnaryCall(
              new MethodHandlers<
                com.ui.cef_control.grpc.gen.OpenPageRequest,
                com.ui.cef_control.grpc.gen.OpenPageResponse>(
                  this, METHODID_OPEN_PAGE)))
          .addMethod(
            METHOD_PAGE_STATUS,
            asyncUnaryCall(
              new MethodHandlers<
                com.ui.cef_control.grpc.gen.PageStatusRequest,
                com.ui.cef_control.grpc.gen.PageStatusResponse>(
                  this, METHODID_PAGE_STATUS)))
          .addMethod(
            METHOD_SHUTDOWN,
            asyncUnaryCall(
              new MethodHandlers<
                com.ui.cef_control.grpc.gen.ShutdownRequest,
                com.ui.cef_control.grpc.gen.ShutdownResponse>(
                  this, METHODID_SHUTDOWN)))
          .build();
    }
  }

  /**
   * <pre>
   **
   * Phase-6 gRPC Service
   * Single unary RPC per command type.
   * CEF is client. Java is server.
   * All methods are unary (request ? response).
   * No streaming, no bidirectional communication at gRPC level.
   * Phase-7 TODO: Consider adding streaming for high-frequency events (metrics, logs).
   * </pre>
   */
  public static final class CefControlServiceStub extends io.grpc.stub.AbstractStub<CefControlServiceStub> {
    private CefControlServiceStub(io.grpc.Channel channel) {
      super(channel);
    }

    private CefControlServiceStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CefControlServiceStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new CefControlServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     **
     * Handshake RPC.
     * CEF initiates this on connection.
     * Java validates and accepts/rejects the session.
     * This MUST be the first RPC called by CEF.
     * </pre>
     */
    public void handshake(com.ui.cef_control.grpc.gen.HandshakeRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.HandshakeResponse> responseObserver) {
      asyncUnaryCall(
          getChannel().newCall(METHOD_HANDSHAKE, getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     **
     * Open Page RPC.
     * Java sends OpenPageRequest.
     * CEF responds immediately with OpenPageResponse (accepted or rejected).
     * CEF then asynchronously sends PageStatusNotification messages
     * (via a separate callback mechanism or polling).
     * Phase-7 TODO: Implement notification delivery (polling or push).
     * </pre>
     */
    public void openPage(com.ui.cef_control.grpc.gen.OpenPageRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.OpenPageResponse> responseObserver) {
      asyncUnaryCall(
          getChannel().newCall(METHOD_OPEN_PAGE, getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     **
     * Query Page Status RPC.
     * Java sends PageStatusRequest to CEF.
     * CEF responds immediately with current PageStatusResponse.
     * Used to poll current page load state synchronously.
     * Phase-7 TODO: Add polling mechanism for continuous updates.
     * </pre>
     */
    public void pageStatus(com.ui.cef_control.grpc.gen.PageStatusRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.PageStatusResponse> responseObserver) {
      asyncUnaryCall(
          getChannel().newCall(METHOD_PAGE_STATUS, getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     **
     * Shutdown RPC.
     * Java instructs CEF to terminate.
     * Phase-7 TODO: Not required for MVP.
     * </pre>
     */
    public void shutdown(com.ui.cef_control.grpc.gen.ShutdownRequest request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.ShutdownResponse> responseObserver) {
      asyncUnaryCall(
          getChannel().newCall(METHOD_SHUTDOWN, getCallOptions()), request, responseObserver);
    }
  }

  /**
   * <pre>
   **
   * Phase-6 gRPC Service
   * Single unary RPC per command type.
   * CEF is client. Java is server.
   * All methods are unary (request ? response).
   * No streaming, no bidirectional communication at gRPC level.
   * Phase-7 TODO: Consider adding streaming for high-frequency events (metrics, logs).
   * </pre>
   */
  public static final class CefControlServiceBlockingStub extends io.grpc.stub.AbstractStub<CefControlServiceBlockingStub> {
    private CefControlServiceBlockingStub(io.grpc.Channel channel) {
      super(channel);
    }

    private CefControlServiceBlockingStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CefControlServiceBlockingStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new CefControlServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     **
     * Handshake RPC.
     * CEF initiates this on connection.
     * Java validates and accepts/rejects the session.
     * This MUST be the first RPC called by CEF.
     * </pre>
     */
    public com.ui.cef_control.grpc.gen.HandshakeResponse handshake(com.ui.cef_control.grpc.gen.HandshakeRequest request) {
      return blockingUnaryCall(
          getChannel(), METHOD_HANDSHAKE, getCallOptions(), request);
    }

    /**
     * <pre>
     **
     * Open Page RPC.
     * Java sends OpenPageRequest.
     * CEF responds immediately with OpenPageResponse (accepted or rejected).
     * CEF then asynchronously sends PageStatusNotification messages
     * (via a separate callback mechanism or polling).
     * Phase-7 TODO: Implement notification delivery (polling or push).
     * </pre>
     */
    public com.ui.cef_control.grpc.gen.OpenPageResponse openPage(com.ui.cef_control.grpc.gen.OpenPageRequest request) {
      return blockingUnaryCall(
          getChannel(), METHOD_OPEN_PAGE, getCallOptions(), request);
    }

    /**
     * <pre>
     **
     * Query Page Status RPC.
     * Java sends PageStatusRequest to CEF.
     * CEF responds immediately with current PageStatusResponse.
     * Used to poll current page load state synchronously.
     * Phase-7 TODO: Add polling mechanism for continuous updates.
     * </pre>
     */
    public com.ui.cef_control.grpc.gen.PageStatusResponse pageStatus(com.ui.cef_control.grpc.gen.PageStatusRequest request) {
      return blockingUnaryCall(
          getChannel(), METHOD_PAGE_STATUS, getCallOptions(), request);
    }

    /**
     * <pre>
     **
     * Shutdown RPC.
     * Java instructs CEF to terminate.
     * Phase-7 TODO: Not required for MVP.
     * </pre>
     */
    public com.ui.cef_control.grpc.gen.ShutdownResponse shutdown(com.ui.cef_control.grpc.gen.ShutdownRequest request) {
      return blockingUnaryCall(
          getChannel(), METHOD_SHUTDOWN, getCallOptions(), request);
    }
  }

  /**
   * <pre>
   **
   * Phase-6 gRPC Service
   * Single unary RPC per command type.
   * CEF is client. Java is server.
   * All methods are unary (request ? response).
   * No streaming, no bidirectional communication at gRPC level.
   * Phase-7 TODO: Consider adding streaming for high-frequency events (metrics, logs).
   * </pre>
   */
  public static final class CefControlServiceFutureStub extends io.grpc.stub.AbstractStub<CefControlServiceFutureStub> {
    private CefControlServiceFutureStub(io.grpc.Channel channel) {
      super(channel);
    }

    private CefControlServiceFutureStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CefControlServiceFutureStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new CefControlServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     **
     * Handshake RPC.
     * CEF initiates this on connection.
     * Java validates and accepts/rejects the session.
     * This MUST be the first RPC called by CEF.
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ui.cef_control.grpc.gen.HandshakeResponse> handshake(
        com.ui.cef_control.grpc.gen.HandshakeRequest request) {
      return futureUnaryCall(
          getChannel().newCall(METHOD_HANDSHAKE, getCallOptions()), request);
    }

    /**
     * <pre>
     **
     * Open Page RPC.
     * Java sends OpenPageRequest.
     * CEF responds immediately with OpenPageResponse (accepted or rejected).
     * CEF then asynchronously sends PageStatusNotification messages
     * (via a separate callback mechanism or polling).
     * Phase-7 TODO: Implement notification delivery (polling or push).
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ui.cef_control.grpc.gen.OpenPageResponse> openPage(
        com.ui.cef_control.grpc.gen.OpenPageRequest request) {
      return futureUnaryCall(
          getChannel().newCall(METHOD_OPEN_PAGE, getCallOptions()), request);
    }

    /**
     * <pre>
     **
     * Query Page Status RPC.
     * Java sends PageStatusRequest to CEF.
     * CEF responds immediately with current PageStatusResponse.
     * Used to poll current page load state synchronously.
     * Phase-7 TODO: Add polling mechanism for continuous updates.
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ui.cef_control.grpc.gen.PageStatusResponse> pageStatus(
        com.ui.cef_control.grpc.gen.PageStatusRequest request) {
      return futureUnaryCall(
          getChannel().newCall(METHOD_PAGE_STATUS, getCallOptions()), request);
    }

    /**
     * <pre>
     **
     * Shutdown RPC.
     * Java instructs CEF to terminate.
     * Phase-7 TODO: Not required for MVP.
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ui.cef_control.grpc.gen.ShutdownResponse> shutdown(
        com.ui.cef_control.grpc.gen.ShutdownRequest request) {
      return futureUnaryCall(
          getChannel().newCall(METHOD_SHUTDOWN, getCallOptions()), request);
    }
  }

  private static final int METHODID_HANDSHAKE = 0;
  private static final int METHODID_OPEN_PAGE = 1;
  private static final int METHODID_PAGE_STATUS = 2;
  private static final int METHODID_SHUTDOWN = 3;

  private static class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final CefControlServiceImplBase serviceImpl;
    private final int methodId;

    public MethodHandlers(CefControlServiceImplBase serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_HANDSHAKE:
          serviceImpl.handshake((com.ui.cef_control.grpc.gen.HandshakeRequest) request,
              (io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.HandshakeResponse>) responseObserver);
          break;
        case METHODID_OPEN_PAGE:
          serviceImpl.openPage((com.ui.cef_control.grpc.gen.OpenPageRequest) request,
              (io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.OpenPageResponse>) responseObserver);
          break;
        case METHODID_PAGE_STATUS:
          serviceImpl.pageStatus((com.ui.cef_control.grpc.gen.PageStatusRequest) request,
              (io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.PageStatusResponse>) responseObserver);
          break;
        case METHODID_SHUTDOWN:
          serviceImpl.shutdown((com.ui.cef_control.grpc.gen.ShutdownRequest) request,
              (io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.ShutdownResponse>) responseObserver);
          break;
        default:
          throw new AssertionError();
      }
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public io.grpc.stub.StreamObserver<Req> invoke(
        io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        default:
          throw new AssertionError();
      }
    }
  }

  private static final class CefControlServiceDescriptorSupplier implements io.grpc.protobuf.ProtoFileDescriptorSupplier {
    @java.lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return com.ui.cef_control.grpc.gen.CefControlProto.getDescriptor();
    }
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (CefControlServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .setSchemaDescriptor(new CefControlServiceDescriptorSupplier())
              .addMethod(METHOD_HANDSHAKE)
              .addMethod(METHOD_OPEN_PAGE)
              .addMethod(METHOD_PAGE_STATUS)
              .addMethod(METHOD_SHUTDOWN)
              .build();
        }
      }
    }
    return result;
  }
}
