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
 * Phase-6.3 Status Callback Service
 * This service is IMPLEMENTED BY JAVA and CALLED BY CEF.
 * Reverses the normal client/server roles for status notifications.
 * CEF calls this service to push status updates to Java asynchronously.
 * Java logs and stores these status events.
 * Phase-7 TODO: Add streaming for high-frequency status updates.
 * </pre>
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.1.2)",
    comments = "Source: cef_service.proto")
public class CefStatusCallbackServiceGrpc {

  private CefStatusCallbackServiceGrpc() {}

  public static final String SERVICE_NAME = "cefcontrol.CefStatusCallbackService";

  // Static method descriptors that strictly reflect the proto.
  @io.grpc.ExperimentalApi("https://github.com/grpc/grpc-java/issues/1901")
  public static final io.grpc.MethodDescriptor<com.ui.cef_control.grpc.gen.PageStatusNotification,
      com.ui.cef_control.grpc.gen.StatusAck> METHOD_NOTIFY_PAGE_STATUS =
      io.grpc.MethodDescriptor.create(
          io.grpc.MethodDescriptor.MethodType.UNARY,
          generateFullMethodName(
              "cefcontrol.CefStatusCallbackService", "NotifyPageStatus"),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.PageStatusNotification.getDefaultInstance()),
          io.grpc.protobuf.ProtoUtils.marshaller(com.ui.cef_control.grpc.gen.StatusAck.getDefaultInstance()));

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static CefStatusCallbackServiceStub newStub(io.grpc.Channel channel) {
    return new CefStatusCallbackServiceStub(channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static CefStatusCallbackServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    return new CefStatusCallbackServiceBlockingStub(channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary and streaming output calls on the service
   */
  public static CefStatusCallbackServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    return new CefStatusCallbackServiceFutureStub(channel);
  }

  /**
   * <pre>
   **
   * Phase-6.3 Status Callback Service
   * This service is IMPLEMENTED BY JAVA and CALLED BY CEF.
   * Reverses the normal client/server roles for status notifications.
   * CEF calls this service to push status updates to Java asynchronously.
   * Java logs and stores these status events.
   * Phase-7 TODO: Add streaming for high-frequency status updates.
   * </pre>
   */
  public static abstract class CefStatusCallbackServiceImplBase implements io.grpc.BindableService {

    /**
     * <pre>
     **
     * Notify Page Status RPC.
     * CEF sends PageStatusNotification to Java.
     * Java responds with StatusAck.
     * Called by CEF when:
     * - Page starts loading (status="LOADING")
     * - Page finishes loading (status="LOADED")
     * - Page encounters error (status="ERROR")
     * - Browser is closing (status="SHUTDOWN")
     * Java MUST NOT block or perform long operations in this handler.
     * Java MUST log the status and return immediately.
     * Phase-7 TODO: Add authentication and encryption.
     * </pre>
     */
    public void notifyPageStatus(com.ui.cef_control.grpc.gen.PageStatusNotification request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.StatusAck> responseObserver) {
      asyncUnimplementedUnaryCall(METHOD_NOTIFY_PAGE_STATUS, responseObserver);
    }

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
          .addMethod(
            METHOD_NOTIFY_PAGE_STATUS,
            asyncUnaryCall(
              new MethodHandlers<
                com.ui.cef_control.grpc.gen.PageStatusNotification,
                com.ui.cef_control.grpc.gen.StatusAck>(
                  this, METHODID_NOTIFY_PAGE_STATUS)))
          .build();
    }
  }

  /**
   * <pre>
   **
   * Phase-6.3 Status Callback Service
   * This service is IMPLEMENTED BY JAVA and CALLED BY CEF.
   * Reverses the normal client/server roles for status notifications.
   * CEF calls this service to push status updates to Java asynchronously.
   * Java logs and stores these status events.
   * Phase-7 TODO: Add streaming for high-frequency status updates.
   * </pre>
   */
  public static final class CefStatusCallbackServiceStub extends io.grpc.stub.AbstractStub<CefStatusCallbackServiceStub> {
    private CefStatusCallbackServiceStub(io.grpc.Channel channel) {
      super(channel);
    }

    private CefStatusCallbackServiceStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CefStatusCallbackServiceStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new CefStatusCallbackServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     **
     * Notify Page Status RPC.
     * CEF sends PageStatusNotification to Java.
     * Java responds with StatusAck.
     * Called by CEF when:
     * - Page starts loading (status="LOADING")
     * - Page finishes loading (status="LOADED")
     * - Page encounters error (status="ERROR")
     * - Browser is closing (status="SHUTDOWN")
     * Java MUST NOT block or perform long operations in this handler.
     * Java MUST log the status and return immediately.
     * Phase-7 TODO: Add authentication and encryption.
     * </pre>
     */
    public void notifyPageStatus(com.ui.cef_control.grpc.gen.PageStatusNotification request,
        io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.StatusAck> responseObserver) {
      asyncUnaryCall(
          getChannel().newCall(METHOD_NOTIFY_PAGE_STATUS, getCallOptions()), request, responseObserver);
    }
  }

  /**
   * <pre>
   **
   * Phase-6.3 Status Callback Service
   * This service is IMPLEMENTED BY JAVA and CALLED BY CEF.
   * Reverses the normal client/server roles for status notifications.
   * CEF calls this service to push status updates to Java asynchronously.
   * Java logs and stores these status events.
   * Phase-7 TODO: Add streaming for high-frequency status updates.
   * </pre>
   */
  public static final class CefStatusCallbackServiceBlockingStub extends io.grpc.stub.AbstractStub<CefStatusCallbackServiceBlockingStub> {
    private CefStatusCallbackServiceBlockingStub(io.grpc.Channel channel) {
      super(channel);
    }

    private CefStatusCallbackServiceBlockingStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CefStatusCallbackServiceBlockingStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new CefStatusCallbackServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     **
     * Notify Page Status RPC.
     * CEF sends PageStatusNotification to Java.
     * Java responds with StatusAck.
     * Called by CEF when:
     * - Page starts loading (status="LOADING")
     * - Page finishes loading (status="LOADED")
     * - Page encounters error (status="ERROR")
     * - Browser is closing (status="SHUTDOWN")
     * Java MUST NOT block or perform long operations in this handler.
     * Java MUST log the status and return immediately.
     * Phase-7 TODO: Add authentication and encryption.
     * </pre>
     */
    public com.ui.cef_control.grpc.gen.StatusAck notifyPageStatus(com.ui.cef_control.grpc.gen.PageStatusNotification request) {
      return blockingUnaryCall(
          getChannel(), METHOD_NOTIFY_PAGE_STATUS, getCallOptions(), request);
    }
  }

  /**
   * <pre>
   **
   * Phase-6.3 Status Callback Service
   * This service is IMPLEMENTED BY JAVA and CALLED BY CEF.
   * Reverses the normal client/server roles for status notifications.
   * CEF calls this service to push status updates to Java asynchronously.
   * Java logs and stores these status events.
   * Phase-7 TODO: Add streaming for high-frequency status updates.
   * </pre>
   */
  public static final class CefStatusCallbackServiceFutureStub extends io.grpc.stub.AbstractStub<CefStatusCallbackServiceFutureStub> {
    private CefStatusCallbackServiceFutureStub(io.grpc.Channel channel) {
      super(channel);
    }

    private CefStatusCallbackServiceFutureStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CefStatusCallbackServiceFutureStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new CefStatusCallbackServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     **
     * Notify Page Status RPC.
     * CEF sends PageStatusNotification to Java.
     * Java responds with StatusAck.
     * Called by CEF when:
     * - Page starts loading (status="LOADING")
     * - Page finishes loading (status="LOADED")
     * - Page encounters error (status="ERROR")
     * - Browser is closing (status="SHUTDOWN")
     * Java MUST NOT block or perform long operations in this handler.
     * Java MUST log the status and return immediately.
     * Phase-7 TODO: Add authentication and encryption.
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ui.cef_control.grpc.gen.StatusAck> notifyPageStatus(
        com.ui.cef_control.grpc.gen.PageStatusNotification request) {
      return futureUnaryCall(
          getChannel().newCall(METHOD_NOTIFY_PAGE_STATUS, getCallOptions()), request);
    }
  }

  private static final int METHODID_NOTIFY_PAGE_STATUS = 0;

  private static class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final CefStatusCallbackServiceImplBase serviceImpl;
    private final int methodId;

    public MethodHandlers(CefStatusCallbackServiceImplBase serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_NOTIFY_PAGE_STATUS:
          serviceImpl.notifyPageStatus((com.ui.cef_control.grpc.gen.PageStatusNotification) request,
              (io.grpc.stub.StreamObserver<com.ui.cef_control.grpc.gen.StatusAck>) responseObserver);
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

  private static final class CefStatusCallbackServiceDescriptorSupplier implements io.grpc.protobuf.ProtoFileDescriptorSupplier {
    @java.lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return com.ui.cef_control.grpc.gen.CefControlProto.getDescriptor();
    }
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (CefStatusCallbackServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .setSchemaDescriptor(new CefStatusCallbackServiceDescriptorSupplier())
              .addMethod(METHOD_NOTIFY_PAGE_STATUS)
              .build();
        }
      }
    }
    return result;
  }
}
