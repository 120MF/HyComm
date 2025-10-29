#include "EchoBackend.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <thread>
#include <format>
#include <HyComm/IpcShared/EchoRequest.hpp>
#include <HyComm/IpcShared/Response.hpp>
#include <HyComm/Common/FdTransfer.hpp>

namespace hy::daemon
{
    EchoBackend::EchoBackend() = default;

    ipc::Response EchoBackend::handle_request(const ipc::Request& req)
    {
        if (auto* open_req = std::get_if<ipc::EchoOpenRequest>(&req))
        {
            return handle_open(*open_req);
        }

        if (auto* config_req = std::get_if<ipc::EchoConfigRequest>(&req))
        {
            return handle_config(*config_req);
        }

        if (auto* close_req = std::get_if<ipc::EchoCloseRequest>(&req))
        {
            return handle_close(*close_req);
        }

        return tl::make_unexpected(common::Error{
            common::ErrorCode::InvalidArguments,
            "Unknown request type in EchoBackend"
        });
    }

    ipc::Response EchoBackend::handle_open(const ipc::EchoOpenRequest& req)
    {
        std::lock_guard lock(m_mutex);

        // Check if already open
        if (m_instances.contains(req.echo_id))
        {
            return tl::make_unexpected(common::Error{
                common::ErrorCode::AlreadyOpen,
                std::format("Echo instance already open: {}", req.echo_id)
            });
        }

        // Create a socketpair for bidirectional communication
        EchoInstance instance;
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, instance.fd_pair) == -1)
        {
            return tl::make_unexpected(common::Error{
                common::ErrorCode::InvalidArguments,
                std::format("Failed to create socketpair: {}", strerror(errno))
            });
        }

        const int client_fd = instance.fd_pair[0]; // Client side
        const int server_fd = instance.fd_pair[1]; // Server side (daemon keeps this)

        // Spawn a thread to handle echo operations
        std::thread([server_fd, uppercase = instance.uppercase_mode]() mutable
        {
            char buffer[4096];
            while (true)
            {
                const ssize_t n = read(server_fd, buffer, sizeof(buffer));
                if (n <= 0)
                {
                    break; // Connection closed or error
                }

                // Echo back (optionally uppercase)
                if (uppercase)
                {
                    for (ssize_t i = 0; i < n; ++i)
                    {
                        buffer[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(buffer[i])));
                    }
                }

                write(server_fd, buffer, n);
            }
            close(server_fd);
        }).detach();

        // Send FD to client via UDS
        if (!common::FdTransfer::connect_and_send(req.uds_path, client_fd))
        {
            close(client_fd);
            close(server_fd);
            return tl::make_unexpected(common::Error{
                common::ErrorCode::FdTransferFailed,
                std::format("Failed to send FD via UDS to: {}", req.uds_path)
            });
        }

        m_instances[req.echo_id] = instance;
        return ipc::GenericSuccessResponse{};
    }

    ipc::Response EchoBackend::handle_config(const ipc::EchoConfigRequest& req)
    {
        std::lock_guard lock(m_mutex);

        auto it = m_instances.find(req.echo_id);
        if (it == m_instances.end())
        {
            return tl::make_unexpected(common::Error{
                common::ErrorCode::InterfaceNotFound,
                std::format("Echo instance not found: {}", req.echo_id)
            });
        }

        // Note: This won't affect the already-running thread
        // In a real implementation, you'd need inter-thread communication
        it->second.uppercase_mode = req.uppercase_mode;
        return ipc::GenericSuccessResponse{};
    }

    ipc::Response EchoBackend::handle_close(const ipc::EchoCloseRequest& req)
    {
        std::lock_guard lock(m_mutex);

        auto it = m_instances.find(req.echo_id);
        if (it == m_instances.end())
        {
            return tl::make_unexpected(common::Error{
                common::ErrorCode::InterfaceNotFound,
                std::format("Echo instance not found: {}", req.echo_id)
            });
        }

        // Close the client-side fd (server side will be closed by the thread)
        close(it->second.fd_pair[0]);

        m_instances.erase(it);
        return ipc::GenericSuccessResponse{};
    }
}

