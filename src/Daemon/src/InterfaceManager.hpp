#ifndef HYCOMM_INTERFACEMANAGER_HPP
#define HYCOMM_INTERFACEMANAGER_HPP

#include <HyComm/IpcShared/Request.hpp>
#include <HyComm/IpcShared/Response.hpp>
#include <ankerl/unordered_dense.h>

namespace hy::daemon
{
    class IInterfaceBackend;

    class InterfaceManager
    {
    public:
        InterfaceManager();

        ipc::Response handle_request(const ipc::Request& req);

    private:
        ankerl::unordered_dense::map<size_t, std::shared_ptr<IInterfaceBackend>> m_backend_registry;
    };
}

#endif //HYCOMM_INTERFACEMANAGER_HPP
