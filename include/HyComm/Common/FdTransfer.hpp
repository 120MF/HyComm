#ifndef HYCOMM_FDTRANSFER_HPP
#define HYCOMM_FDTRANSFER_HPP

#include <string>
#include <tl/expected.hpp>
#include "Error.hpp"

namespace hy::common
{
    class FdTransfer
    {
    public:
        FdTransfer(const FdTransfer&) = delete;
        FdTransfer& operator=(const FdTransfer&) = delete;

        FdTransfer(FdTransfer&& other) noexcept;
        FdTransfer& operator=(FdTransfer&& other) noexcept;
        ~FdTransfer();

        static tl::expected<FdTransfer, Error> create_listener();

        [[nodiscard]] std::string_view get_socket_path() const;

        [[nodiscard]] tl::expected<int, Error> accept_and_receive() noexcept;

        static tl::expected<void, Error> connect_and_send(
            const std::string& socket_path,
            int fd_to_send) noexcept;

    private:
        int m_listen_socket = -1;
        std::string m_socket_path;

        explicit FdTransfer(int listen_fd, std::string socket_path) noexcept;

        void cleanup() noexcept;
    };
}

#endif //HYCOMM_FDTRANSFER_HPP
