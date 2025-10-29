#ifndef HYCOMM_SERIALBUILDER_HPP
#define HYCOMM_SERIALBUILDER_HPP

#include <memory>
#include <string>

#include "HyComm/Configs/SerialConfig.hpp"
#include "HyComm/Detail/InterfaceHandle.hpp"
#include "HyComm/IpcShared/SerialRequest.hpp"

namespace hy
{
    class Manager;
    namespace interfaces { class Serial; }

    class SerialBuilder
    {
    public:
        explicit SerialBuilder(Manager* manager) : m_manager(manager) {}

        SerialBuilder& device(std::string path)
        {
            m_config.device_path = std::move(path);
            return *this;
        }

        SerialBuilder& baud_rate(uint32_t rate)
        {
            m_config.baud_rate = rate;
            return *this;
        }

        SerialBuilder& data_bits(ipc::DataBits bits)
        {
            m_config.data_bits = bits;
            return *this;
        }

        SerialBuilder& stop_bits(ipc::StopBits bits)
        {
            m_config.stop_bits = bits;
            return *this;
        }

        SerialBuilder& parity(ipc::Parity p)
        {
            m_config.parity = p;
            return *this;
        }

        SerialBuilder& flow_control(ipc::FlowControl fc)
        {
            m_config.flow_control = fc;
            return *this;
        }

        SerialBuilder& rts_dtr(bool enabled)
        {
            m_config.rts_dtr_on = enabled;
            return *this;
        }

        InterfaceHandle<interfaces::Serial> build();

    private:
        Manager* m_manager;
        SerialConfig m_config;
    };
}

#endif //HYCOMM_SERIALBUILDER_HPP
