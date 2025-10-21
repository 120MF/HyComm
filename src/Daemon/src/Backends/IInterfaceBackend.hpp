#ifndef HYCOMM_IINTERFACEBACKEND_HPP
#define HYCOMM_IINTERFACEBACKEND_HPP
#include <HyComm/IpcShared/Response.hpp>

#include "HyComm/IpcShared/Request.hpp"

namespace hy::daemon
{
    class IInterfaceBackend
    {
    public:
        virtual ~IInterfaceBackend() = default;
        virtual ipc::Response handle_request(const ipc::Request& req) = 0;
    };
}

#endif //HYCOMM_IINTERFACEBACKEND_HPP
