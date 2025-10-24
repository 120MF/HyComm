#ifndef HYCOMM_REQUEST_HPP
#define HYCOMM_REQUEST_HPP
#include <string>
#include <variant>
#include <cstdint>
#include "SerialRequest.hpp"

namespace hy::ipc
{
    // 打开接口的请求类型（后期可扩展：CANOpenRequest 等）
    using OpenRequest = std::variant<
        SerialOpenRequest
    >;

    // 通用请求类型
    using Request = std::variant<
        SerialOpenRequest,
        SerialConfigRequest,
        SerialCloseRequest
    >;
}

#endif //HYCOMM_REQUEST_HPP
