#ifndef HYCOMM_COMMON_ERROR_HPP
#define HYCOMM_COMMON_ERROR_HPP
#include <string>

namespace hy::common
{
    enum class ErrorCode
    {
        SocketCreationFailed,
        SocketBindFailed,
        SocketListenFailed,
        SocketConnectFailed,
        SocketAcceptFailed,
        MessageSendFailed,
        MessageReceiveFailed,
        InvalidFdReceived,
    };

    struct Error
    {
        ErrorCode code;
        std::string message;
    };
}

#endif //HYCOMM_COMMON_ERROR_HPP
