#pragma once

#include <string>
#include <memory>
#include "IpcProtocolException.h"

namespace cef_ui {
	namespace ipc {

		/// Abstract message channel interface for sending/receiving IPC messages.
		/// Allows decoupling message logic from transport (sockets, pipes, etc).
		/// 
		/// Phase 3: No networking yet. This is the contract for later transport implementation.
		class IMessageChannel {
		public:
			virtual ~IMessageChannel() = default;

			/// Send a message string to the remote endpoint.
			/// @param message JSON message string
			/// @throws IpcProtocolException if send fails
			virtual void Send(const std::string& message) = 0;

			/// Receive a message string from the remote endpoint.
			/// Blocking call - returns when message available or error occurs.
			/// @return JSON message string
			/// @throws IpcProtocolException if receive fails
			virtual std::string Receive() = 0;

			/// Check if the channel is connected/open.
			/// @return true if channel is ready for send/receive
			virtual bool IsConnected() const = 0;

			/// Close the channel.
			virtual void Close() = 0;
		};

	}  // namespace ipc
}  // namespace cef_ui
