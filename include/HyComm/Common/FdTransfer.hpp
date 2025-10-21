#ifndef HYCOMM_FDTRANSFER_HPP
#define HYCOMM_FDTRANSFER_HPP

#include <string>
#include <tl/expected.hpp>
#include "Error.hpp"

namespace hy::common
{
    struct UDSPacket
    {
        std::string path;
        int sock_fd;
    };

    class FdTransfer
    {
    public:
        static tl::expected<std::string, Error> create_listener();
        static int receive_fd(int listening_socket);
        static bool send_fd(const std::string& uds_path, int fd_to_send);
    };
}

#endif //HYCOMM_FDTRANSFER_HPP
