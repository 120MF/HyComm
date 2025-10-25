#ifndef HYCOMM_INTERFACEMANAGER_HPP
#define HYCOMM_INTERFACEMANAGER_HPP

#include <HyComm/IpcShared/Request.hpp>
#include <HyComm/IpcShared/Response.hpp>
#include <ankerl/unordered_dense.h>
#include <memory>

namespace hy::daemon
{
    class IInterfaceBackend;

    class InterfaceManager
    {
    public:
        InterfaceManager();

        ipc::Response handle_request(const ipc::Request& req);

    private:
        // 为每种请求类型注册对应的 backend
        template <typename RequestT>
        void register_backend_for_request(std::shared_ptr<IInterfaceBackend> backend)
        {
            m_backend_registry[typeid(RequestT).hash_code()] = backend;
        }

        ankerl::unordered_dense::map<size_t, std::shared_ptr<IInterfaceBackend>> m_backend_registry;
    };
}

#endif //HYCOMM_INTERFACEMANAGER_HPP
