#include "InterfaceManager.hpp"

#include "Backends/SerialBackend.hpp"
#include <HyComm/IpcShared/SerialRequest.hpp>

hy::daemon::InterfaceManager::InterfaceManager()
{
    const auto serial_backend = std::make_shared<SerialBackend>();

    register_backend_for_request<ipc::SerialOpenRequest>(serial_backend);
    register_backend_for_request<ipc::SerialConfigRequest>(serial_backend);
    register_backend_for_request<ipc::SerialCloseRequest>(serial_backend);
}

hy::ipc::Response hy::daemon::InterfaceManager::handle_request(const ipc::Request& req)
{
    return std::visit([this, &req]<typename T0>(T0&& arg) -> ipc::Response
    {
        using T = std::decay_t<T0>;
        const auto it = m_backend_registry.find(typeid(T).hash_code());
        if (it == m_backend_registry.end())
        {
            return tl::make_unexpected(
                common::Error{common::ErrorCode::InvalidArguments, "No backend for this request"});
        }

        return it->second->handle_request(req);
    }, req);
}
