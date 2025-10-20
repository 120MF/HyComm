#include <iox2/iceoryx2.hpp>

constexpr iox::units::Duration CYCLE_TIME = iox::units::Duration::fromSeconds(1);

int main()
{
    using namespace iox2;
    set_log_level_from_env_or(LogLevel::Info);
    const auto node = NodeBuilder().create<ServiceType::Ipc>().expect("node");
    const auto service = node.service_builder(ServiceName::create("HyCommDaemon").expect("name"))
                             .event()
                             .open_or_create()
                             .expect("service");
    auto listener = service.listener_builder().create().expect("listener");
    std::cout << "Listener ready to receive events!\n";
    while (node.wait(iox::units::Duration::zero()).has_value())
    {
        listener.timed_wait_one(CYCLE_TIME).and_then([](auto maybe_event_id)
        {
            maybe_event_id.and_then([](auto event_id)
            {
                std::cout << "event was triggered with id: " << event_id << '\n';
            });
        });
    }
    return 0;
}
