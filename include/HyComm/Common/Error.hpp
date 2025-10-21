#ifndef HYCOMM_COMMON_ERROR_HPP
#define HYCOMM_COMMON_ERROR_HPP
#include <string>

namespace hy::common
{
    enum class ErrorCode
    {
        UnknownError,

        SocketCreationFailed,
        SocketBindFailed,
        SocketListenFailed,
        SocketConnectFailed,
        SocketAcceptFailed,
        MessageSendFailed,
        MessageReceiveFailed,
        InvalidFdReceived,

        PermissionDenied,
        InterfaceNotFound,
        InvalidArguments,
        AlreadyOpen,
        ResourceCreationFailed, // e.g., socket() failed
        FdTransferFailed, // UDS an FD failed to send
        InternalDaemonError,
    };

    struct Error
    {
        ErrorCode code;
        std::string message;
    };
}

#endif //HYCOMM_COMMON_ERROR_HPP
