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
                  .open_or_create().expect("")
                  .server_builder().create().expect("");
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
        while (m_listener.blocking_wait_all([this]([[maybe_unused]] auto event_id)
        {
            while (true)
            {
                auto active_request = m_server.receive().expect("");
                if (!active_request.has_value())
                {
                    break; // 处理完成，等待下一次listener调用
                }
                auto response = active_request->loan_uninit().expect("");
                auto initialized_response = response.write_payload([&]
                {
                    return m_request_handler(active_request.value().payload());
                }());
                iox2::send(std::move(initialized_response)).expect("");
            }
        }));
    }
}
