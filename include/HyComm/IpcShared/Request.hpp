#ifndef HYCOMM_REQUEST_HPP
#define HYCOMM_REQUEST_HPP
#include <string>
#include <variant>
#include <cstdint>
#include "SerialRequest.hpp"

namespace hy::ipc
{
    using Request = std::variant<
        SerialOpenRequest,
        SerialConfigRequest,
        SerialCloseRequest
    >;
}

#endif //HYCOMM_REQUEST_HPP
