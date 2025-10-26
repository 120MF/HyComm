#ifndef HYCOMM_URINGMANAGER_HPP
#define HYCOMM_URINGMANAGER_HPP

#include <liburing.h>
#include <functional>
#include <memory>
#include <map>
#include <span>


namespace hy::detail
{
    // 使用ssize_t来表示I/O操作的结果，与read/write的返回值保持一致
    using IoResult = ssize_t;

    // 定义两种回调的类型签名
    using ReadCallback = std::function<void(IoResult bytes_read)>;
    using WriteCallback = std::function<void(IoResult bytes_written)>;

    class UringManager : public std::enable_shared_from_this<UringManager>
    {
    public:
        // 工厂函数，推荐的创建方式
        static std::shared_ptr<UringManager> create(unsigned int queue_depth = 256);

        // 析构函数负责清理io_uring
        ~UringManager();

        // 禁止拷贝和移动，因为它管理着一个唯一的内核资源
        UringManager(const UringManager&) = delete;
        UringManager& operator=(const UringManager&) = delete;

        /**
         * @brief 提交一个异步读请求
         * @param fd 文件描述符
         * @param buffer 用于接收数据的缓冲区
         * @param callback 操作完成时调用的回调函数
         */
        void submit_read(int fd, std::span<std::byte> buffer, ReadCallback callback);

        /**
         * @brief 提交一个异步写请求
         * @param fd 文件描述符
         * @param buffer 要发送的数据
         * @param callback 操作完成时调用的回调函数
         */
        void submit_write(int fd, std::span<const std::byte> buffer, WriteCallback callback);

        /**
         * @brief 启动事件循环。此函数将阻塞，直到stop()被调用。
         */
        void run();

        /**
         * @brief 停止事件循环。可以从另一个线程调用此函数。
         */
        void stop();

    private:
        // 私有构造函数，强制使用create()工厂
        UringManager();

        // 通用的完成回调类型，它接收整个CQE作为参数
        using CompletionCallback = std::function<void(const io_uring_cqe*)>;

        // 初始化io_uring实例
        bool initialize(unsigned int queue_depth);

        // 提交一个请求并注册其回调
        void submit_request(struct io_uring_sqe* sqe, CompletionCallback callback);

        io_uring m_ring{};
        std::atomic<bool> m_is_running{false};

        // 用于保护m_active_requests的互斥锁
        std::mutex m_request_mutex;
        uint64_t m_next_request_id = 1;
        std::map<uint64_t, CompletionCallback> m_active_requests;
    };
}

#endif //HYCOMM_URINGMANAGER_HPP
