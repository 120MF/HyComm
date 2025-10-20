#ifndef HYCOMM_DAEMONNODE_HPP
#define HYCOMM_DAEMONNODE_HPP

#include <HyComm/IpcShared/Response.hpp>
#include <HyComm/IpcShared/Request.hpp>

#include <iox2/service.hpp>
#include <iox2/node.hpp>

namespace hy::daemon
{
    class DaemonNode
    {
    public:
        using RequestHandler = std::function<ipc::Response(const ipc::Request&)>;
        DaemonNode(const RequestHandler& handler);
        void run();

    private:
        iox2::Node<iox2::ServiceType::Ipc> m_node;
        iox2::PortFactoryRequestResponse<iox2::ServiceType::Ipc, ipc::Request, void, ipc::Response, void> m_server;
        iox2::Listener<iox2::ServiceType::Ipc> m_listener;
        RequestHandler m_request_handler;
    };
}

#endif //HYCOMM_DAEMONNODE_HPP
