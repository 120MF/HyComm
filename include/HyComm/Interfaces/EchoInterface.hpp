#ifndef HYCOMM_ECHOINTERFACE_HPP
#define HYCOMM_ECHOINTERFACE_HPP

#include <HyComm/Detail/GenericInterface.hpp>
#include <HyComm/Protocols/EchoTraits.hpp>
#include <HyComm/Configs/EchoConfig.hpp>
#include <functional>

namespace hy
{
    template <typename T>
    class InterfaceHandle;
}

namespace hy::interfaces
{
    class Echo : public detail::GenericInterface<Echo, protocols::EchoTraits>
    {
    public:
        using Traits = protocols::EchoTraits;
        using ConfigType = configs::EchoConfig;

        friend class Manager;
        friend class InterfaceHandle<Echo>;

        // Helper struct to enable make_shared with private constructor
        struct PrivateTag
        {
        };

        // Public constructor that requires PrivateTag (only Manager can create)
        Echo(PrivateTag, const int fd, const std::shared_ptr<detail::UringManager>& uring_manager)
            : GenericInterface(fd, uring_manager)
        {
        }

        void dispatch_received_frame(const FrameType& frame) const
        {
            if (m_callback)
            {
                m_callback(frame);
            }
        }

        void set_callback(std::function<void(const FrameType& frame)> callback)
        {
            m_callback = std::move(callback);
        }

    private:
        std::function<void(const FrameType& frame)> m_callback;
    };
}

namespace hy
{
    using Echo = interfaces::Echo;
}

#endif //HYCOMM_ECHOINTERFACE_HPP

