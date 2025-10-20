#include "iox2/iceoryx2.hpp"
#include <iostream>

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
    const auto max_event_id = service.static_config().event_id_max_value();
    const auto notifier = service.notifier_builder().create().expect("notifier");

    int counter = 0;
    while (node.wait(CYCLE_TIME).has_value())
    {
        counter += 1;
        const auto event_id = EventId(counter % max_event_id);
        notifier.notify_with_custom_event_id(event_id).expect("notify");
        std::cout << "Trigger event with id " << event_id << '\n';
    }
}
