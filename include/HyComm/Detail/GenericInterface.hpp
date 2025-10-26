#ifndef HYCOMM_GENERICINTERFACE_HPP
#define HYCOMM_GENERICINTERFACE_HPP


#include "UringManager.hpp"
#include <functional>
#include <memory>

namespace hy::detail
{
    template <typename Derived, typename Traits>
    class GenericInterface
    {
    public:
        using FrameType = Traits::FrameType;
        using ReceiveCallback = std::function<void(const FrameType&)>;

        void send(const FrameType& frame, WriteCallback& callback)
        {
            auto buffer_view = Traits::serialize(frame);
            m_uring_manager->submit_write(m_fd, buffer_view, callback);
        }

        void on_receive(ReceiveCallback callback)
        {
            m_callback = std::move(callback);
        }

    protected:
        GenericInterface(int fd, std::shared_ptr<UringManager> uring_manager) : m_fd(fd), m_uring_manager(uring_manager)
        {
        }

    private:
        Derived* derived() { return static_cast<Derived*>(this); }
        const Derived* derived() const { return static_cast<const Derived*>(this); }

        void post_continuous_read()
        {
            m_uring_manager->submit_read(m_fd, m_read_buffer, [this](size_t bytes_read)
            {
                if (bytes_read > 0)
                {
                    FrameType frame = Traits::deserialize({m_read_buffer, bytes_read});
                    derived()->dispatch_received_frame(frame);
                }
                post_continuous_read();
            });
        }

        ReceiveCallback m_callback;

        int m_fd;
        std::shared_ptr<UringManager> m_uring_manager;
        std::array<std::byte, 2048> m_read_buffer{};
    };
}

#endif //HYCOMM_GENERICINTERFACE_HPP
