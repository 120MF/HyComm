#ifndef HYCOMM_SERIALINTERFACE_HPP
#define HYCOMM_SERIALINTERFACE_HPP

#include <HyComm/Detail/GenericInterface.hpp>
#include <HyComm/Protocols/SerialTraits.hpp>


namespace hy::interfaces
{
    class Serial : public detail::GenericInterface<Serial, protocols::SerialTraits>
    {
    public:
        using Traits = protocols::SerialTraits;
        friend class Manager;

        Serial(const int fd, const std::shared_ptr<detail::UringManager>& uring_manager) : GenericInterface(
            fd, uring_manager)
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
    using Serial = interfaces::Serial;
}

#endif //HYCOMM_SERIALINTERFACE_HPP
