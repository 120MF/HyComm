#include <HyComm/Builders/SerialBuilder.hpp>
#include <HyComm/Manager.hpp>
#include <HyComm/Interfaces/SerialInterface.hpp>

hy::InterfaceHandle<hy::interfaces::Serial> hy::SerialBuilder::build()
{
    return m_manager->create_interface<interfaces::Serial>(m_config);
}
