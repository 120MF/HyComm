#ifndef HYCOMM_ERROR_HPP
#define HYCOMM_ERROR_HPP

#include <string>

namespace hy::ipc
{
    enum class ErrorCode
    {
        UnknownError,
        PermissionDenied,
        InterfaceNotFound,
        InvalidArguments,
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

#endif //HYCOMM_ERROR_HPP
