#ifndef HYCOMM_SERIALTRAITS_HPP
#define HYCOMM_SERIALTRAITS_HPP
#include <vector>
#include <cstdint>
#include <span>

#include <HyComm/IpcShared/Request.hpp>
#include <HyComm/Configs/SerialConfig.hpp>

namespace hy::protocols
{
    struct SerialTraits
    {
        static constexpr auto name = "Serial";
        using FrameType = std::vector<uint8_t>;

        static ipc::OpenRequest make_open_request(const std::string& device_path, const uint32_t baud_rate = 115200,
                                                  const ipc::DataBits data_bits = ipc::DataBits::BITS_8,
                                                  const ipc::StopBits stop_bits = ipc::StopBits::ONE,
                                                  const ipc::Parity parity = ipc::Parity::NONE,
                                                  const ipc::FlowControl flow_control = ipc::FlowControl::NONE,
                                                  const bool rts_dtr_on = false

        )
        {
            return ipc::SerialOpenRequest{
                device_path, baud_rate, data_bits, stop_bits, parity, flow_control, rts_dtr_on, ""
            };
        }

        static ipc::OpenRequest make_open_request(const configs::SerialConfig& config);

        static ipc::ConfigRequest make_config_request(const configs::SerialConfig& config)
        {
            return ipc::SerialConfigRequest{
                config.device_path,
                config.data_bits,
                config.stop_bits,
                config.parity,
                config.flow_control
            };
        }

        static ipc::CloseRequest make_close_request(const configs::SerialConfig& config)
        {
            return ipc::SerialCloseRequest{config.device_path};
        }

        static FrameType deserialize(const std::span<const std::byte>& buffer)
        {
            FrameType result;
            result.reserve(buffer.size());
            for (const auto& b : buffer)
            {
                result.push_back(static_cast<uint8_t>(b));
            }
            return result;
        }

        static std::span<const std::byte> serialize(const FrameType& data)
        {
            return {reinterpret_cast<const std::byte*>(data.data()), data.size()};
        }
    };
}

#endif //HYCOMM_SERIALTRAITS_HPP
