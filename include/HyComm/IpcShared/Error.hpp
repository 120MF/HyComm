#ifndef HYCOMM_DAEMON_ERROR_HPP
#define HYCOMM_DAEMON_ERROR_HPP

#include <string>

namespace hy::ipc
{
    enum class ErrorCode
    {
        UnknownError,
        PermissionDenied,
        InterfaceNotFound,
        InvalidArguments,
        AlreadyOpen,
        ResourceCreationFailed, // e.g., socket() failed
        FdTransferFailed, // UDS an FD failed to send
        InternalDaemonError,
    };

    struct DaemonError
    {
        ErrorCode code;
        std::string message;
    };
}

#endif //HYCOMM_DAEMON_ERROR_HPP
