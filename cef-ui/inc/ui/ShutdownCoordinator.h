class ShutdownCoordinator {
 public:
  ShutdownCoordinator();

  ~ShutdownCoordinator() noexcept = default;

  ShutdownCoordinator(const ShutdownCoordinator&) = delete;

  ShutdownCoordinator& operator=(const ShutdownCoordinator&) = delete;

  ShutdownCoordinator(ShutdownCoordinator&&) = delete;

  ShutdownCoordinator& operator=(ShutdownCoordinator&&) = delete;

  /// Request application shutdown (idempotent).
  void RequestShutdown() noexcept;

  /// Check if shutdown has been requested.
  bool IsShutdownRequested() const noexcept;

 private:
 
  bool shutdown_requested_;
};
