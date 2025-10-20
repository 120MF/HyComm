#ifndef HYCOMM_REQUEST_HPP
#define HYCOMM_REQUEST_HPP
#include <string>
#include <variant>
#include <cstdint>

namespace hy::ipc
{
    struct SerialOpenRequest
    {
        std::string interface_name;
        uint32_t baud_rate;
    };

    struct SerialCloseRequest
    {
        std::string interface_name;
    };

    using Request = std::variant<
        SerialOpenRequest,
        SerialCloseRequest
    >;
}

#endif //HYCOMM_REQUEST_HPP
