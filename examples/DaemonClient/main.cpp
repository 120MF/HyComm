#include <HyComm/Manager.hpp>
#include <HyComm/Interfaces/SerialInterface.hpp>

int main()
{
    auto manager = hy::Manager::create();
    auto serial_interface = manager->get_interface<hy::Serial>("");
    return 0;
}
