#include <cassert>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <HyComm/Common/FdTransfer.hpp>
#include <sys/socket.h>
#include <sys/un.h>

static std::string generate_unique_socket_path()
{
    const auto now = std::chrono::system_clock::now()
                    .time_since_epoch().count();
    const auto pid = getpid();

    std::string path = "/tmp/hycomm_";
    path += std::to_string(pid);
    path += "_";
    path += std::to_string(now);
    path += ".sock";

    return path;
}

namespace hy::common
{
    FdTransfer::FdTransfer(FdTransfer&& other) noexcept
        : m_listen_socket(other.m_listen_socket),
          m_socket_path(std::move(other.m_socket_path))
    {
        other.m_listen_socket = -1;
        other.m_socket_path.clear();
    }

    FdTransfer& FdTransfer::operator=(FdTransfer&& other) noexcept
    {
        if (this != &other)
        {
            cleanup();

            m_listen_socket = other.m_listen_socket;
            m_socket_path = std::move(other.m_socket_path);

            other.m_listen_socket = -1;
            other.m_socket_path.clear();
        }
        return *this;
    }

    FdTransfer::~FdTransfer()
    {
        cleanup();
    }

    tl::expected<FdTransfer, Error> FdTransfer::create_listener()
    {
        std::string socket_path = generate_unique_socket_path();
        int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd < 0)
        {
            return tl::make_unexpected(Error{
                ErrorCode::SocketCreationFailed,
                std::format("socket(AF_UNIX) failed: {}", strerror(errno))
            });
        }
        sockaddr_un addr = {};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(),
                sizeof(addr.sun_path) - 1);
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

        if (bind(sock_fd, reinterpret_cast<sockaddr*>(&addr),
                 sizeof(sockaddr_un)) < 0)
        {
            close(sock_fd);
            unlink(socket_path.c_str());
            return tl::make_unexpected(Error{
                ErrorCode::SocketBindFailed,
                std::format("bind() failed: {}", strerror(errno))
            });
        }
        if (listen(sock_fd, SOMAXCONN) < 0)
        {
            close(sock_fd);
            unlink(socket_path.c_str());
            return tl::make_unexpected(Error{
                ErrorCode::SocketListenFailed,
                std::string("listen() failed: ") + strerror(errno)
            });
        }
        return FdTransfer(sock_fd, std::move(socket_path));
    }

    std::string_view FdTransfer::get_socket_path() const
    {
        return m_socket_path;
    }

    tl::expected<int, Error> FdTransfer::accept_and_receive() noexcept
    {
        if (m_listen_socket < 0)
        {
            return tl::make_unexpected(Error{
                ErrorCode::SocketAcceptFailed,
                "listen socket is not valid"
            });
        }

        // 等待连接
        sockaddr_un client_addr{};
        socklen_t addr_len = sizeof(client_addr);

        const int conn_fd = accept(m_listen_socket,
                                   reinterpret_cast<struct sockaddr*>(&client_addr),
                                   &addr_len);
        if (conn_fd < 0)
        {
            return tl::make_unexpected(Error{
                ErrorCode::SocketAcceptFailed,
                std::format("accept() failed: {}", strerror(errno))
            });
        }
        // 准备接收缓冲区
        char buffer[256];
        iovec iov[1];
        iov[0].iov_base = buffer;
        iov[0].iov_len = sizeof(buffer);

        // 准备控制数据缓冲区
        union
        {
            cmsghdr cm;
            char control[CMSG_SPACE(sizeof(int))];
        } cmsgu = {};

        // 准备msghdr结构
        msghdr msg = {};
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        // 接收消息
        ssize_t ret = recvmsg(conn_fd, &msg, 0);
        if (ret < 0)
        {
            close(conn_fd);
            return tl::make_unexpected(Error{
                ErrorCode::MessageReceiveFailed,
                std::string("recvmsg() failed: ") + strerror(errno)
            });
        }
        int received_fd = -1;
        const cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);

        if (cmsg != nullptr &&
            cmsg->cmsg_level == SOL_SOCKET &&
            cmsg->cmsg_type == SCM_RIGHTS)
        {
            const auto fd_ptr = reinterpret_cast<const int*>(CMSG_DATA(cmsg));
            received_fd = *fd_ptr;
        }
        close(conn_fd);

        if (received_fd < 0)
        {
            return tl::make_unexpected(Error{
                ErrorCode::InvalidFdReceived,
                "Failed to extract FD from ancillary data"
            });
        }

        return received_fd;
    }

    tl::expected<void, Error> FdTransfer::connect_and_send(const std::string& socket_path,
                                                           const int fd_to_send) noexcept
    {
        // 创建客户端socket
        const int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client_fd < 0)
        {
            return tl::make_unexpected(Error{
                ErrorCode::SocketCreationFailed,
                std::format("socket(AF_UNIX) failed: {}", strerror(errno))
            });
        }

        // 准备Server地址
        sockaddr_un server_addr = {};
        server_addr.sun_family = AF_UNIX;
        strncpy(server_addr.sun_path, socket_path.c_str(),
                sizeof(server_addr.sun_path) - 1);
        server_addr.sun_path[sizeof(server_addr.sun_path) - 1] = '\0';

        // 连接到Server
        if (connect(client_fd, reinterpret_cast<sockaddr*>(&server_addr),
                    sizeof(sockaddr_un)) < 0)
        {
            close(client_fd);
            return tl::make_unexpected(Error{
                ErrorCode::SocketConnectFailed,
                std::format("connect() failed: {}", strerror(errno))
            });
        }
        // 准备发送的数据
        char dummy_byte = 0;
        iovec iov[1];
        iov[0].iov_base = &dummy_byte;
        iov[0].iov_len = 1;

        // 准备控制数据(ancillary data)
        union
        {
            cmsghdr cm;
            char control[CMSG_SPACE(sizeof(int))];
        } cmsgu = {};

        // 准备msghdr结构
        msghdr msg = {};
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);

        // 填充FD
        cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
        assert(cmsg != nullptr);
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        const auto fd_ptr = reinterpret_cast<int*>(CMSG_DATA(cmsg));
        *fd_ptr = fd_to_send;

        // 发送消息
        const ssize_t ret = sendmsg(client_fd, &msg, 0);

        // 关闭连接
        close(client_fd);

        // 检查发送结果
        if (ret < 0)
        {
            return tl::make_unexpected(Error{
                ErrorCode::MessageSendFailed,
                std::format("sendmsg() failed: {}", strerror(errno))
            });
        }

        return {};
    }

    FdTransfer::FdTransfer(const int listen_fd, std::string socket_path) noexcept : m_listen_socket(listen_fd),
        m_socket_path(std::move(socket_path))
    {
    }

    void FdTransfer::cleanup() noexcept
    {
        if (m_listen_socket >= 0)
        {
            close(m_listen_socket);
            m_listen_socket = -1;
        }

        if (!m_socket_path.empty())
        {
            unlink(m_socket_path.c_str());
            m_socket_path.clear();
        }
    }
}
