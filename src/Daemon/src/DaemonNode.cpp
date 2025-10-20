#include "DaemonNode.hpp"

namespace hy::daemon
{
    DaemonNode::DaemonNode(const RequestHandler& handler) :
        m_node([]
        {
            return iox2::NodeBuilder().create<
                iox2::ServiceType::Ipc>().expect("");
        }()),
        m_server([this]
        {
            return m_node
                  .service_builder(iox2::ServiceName::create("HyCommDaemonServer")
                      .expect(""))
                  .request_response<ipc::Request, ipc::Response>()
                  .open_or_create()
                  .expect("");
        }()),
        m_listener([this]
        {
            return m_node
                  .service_builder(iox2::ServiceName::create("HyCommDaemonEvent").expect(""))
                  .event().open_or_create().expect("")
                  .listener_builder().create().expect("");
        }()),
        m_request_handler(handler)
    {
    }

    void DaemonNode::run()
    {
        std::cout << "Daemon Node: Start listening...\n";
        while (m_listener.blocking_wait_all([]([[maybe_unused]] auto event_id)
        {
        }));
    }
}
