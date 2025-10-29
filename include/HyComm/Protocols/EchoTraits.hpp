#ifndef HYCOMM_ECHOTRAITS_HPP
#define HYCOMM_ECHOTRAITS_HPP

#include <vector>
#include <cstdint>
#include <span>
#include <string>

#include <HyComm/IpcShared/Request.hpp>
#include <HyComm/Configs/EchoConfig.hpp>

namespace hy::protocols
{
    struct EchoTraits
    {
        static constexpr auto name = "Echo";
        using FrameType = std::string; // Echo works with strings

        static ipc::OpenRequest make_open_request(const std::string& echo_id)
        {
            return ipc::EchoOpenRequest{echo_id, ""};
        }

        static ipc::OpenRequest make_open_request(const configs::EchoConfig& config)
        {
            return ipc::EchoOpenRequest{config.echo_id, ""};
        }

        static ipc::ConfigRequest make_config_request(const configs::EchoConfig& config)
        {
            return ipc::EchoConfigRequest{config.echo_id, config.uppercase_mode};
        }

        static ipc::CloseRequest make_close_request(const configs::EchoConfig& config)
        {
            return ipc::EchoCloseRequest{config.echo_id};
        }

        static FrameType deserialize(const std::span<const std::byte>& buffer)
        {
            return std::string(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        }

        static std::span<const std::byte> serialize(const FrameType& data)
        {
            return {reinterpret_cast<const std::byte*>(data.data()), data.size()};
        }
    };
}

#endif //HYCOMM_ECHOTRAITS_HPP

