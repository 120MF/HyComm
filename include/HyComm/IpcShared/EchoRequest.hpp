#ifndef HYCOMM_ECHOREQUEST_HPP
#define HYCOMM_ECHOREQUEST_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace hy::ipc
{
    // Echo interface just needs a simple identifier for open/close
    struct EchoOpenRequest
    {
        std::string echo_id; // Unique ID for this echo instance
        std::string uds_path;
    };

    struct EchoConfigRequest
    {
        std::string echo_id;
        bool uppercase_mode; // If true, echo back in uppercase
    };

    struct EchoCloseRequest
    {
        std::string echo_id;
    };
}

#endif //HYCOMM_ECHOREQUEST_HPP

