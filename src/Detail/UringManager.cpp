#include <HyComm/Detail/UringManager.hpp>
#include <iostream>

namespace hy::detail
{
    std::shared_ptr<UringManager> UringManager::create(unsigned int queue_depth)
    {
        struct make_shared_enabler : UringManager
        {
        };
        auto manager = std::make_shared<make_shared_enabler>();

        if (manager->initialize(queue_depth))
        {
            return manager;
        }
        return nullptr; // 初始化失败
    }

    UringManager::UringManager()
    = default;

    UringManager::~UringManager()
    {
        // 确保在析构时，事件循环已经停止
        if (m_is_running.load())
        {
            stop();
        }
        io_uring_queue_exit(&m_ring);
    }

    void UringManager::submit_read(int fd, std::span<std::byte> buffer, ReadCallback callback)
    {
        io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
        if (!sqe)
        {
            std::cerr << "UringManager: Submission queue is full!" << std::endl;
            // 在生产环境中，可能需要更复杂的处理，比如排队或返回错误
            return;
        }

        // 准备一个读操作
        io_uring_prep_read(sqe, fd, buffer.data(), buffer.size(), -1);

        // 创建一个通用的完成回调，它捕获用户提供的具体回调
        CompletionCallback completion_cb = [cb = std::move(callback)](const io_uring_cqe* cqe)
        {
            // cqe->res 包含了read操作的返回值（读取的字节数或负数错误码）
            cb(cqe->res);
        };

        submit_request(sqe, std::move(completion_cb));
    }

    void UringManager::submit_write(int fd, std::span<const std::byte> buffer, WriteCallback callback)
    {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&m_ring);
        if (!sqe)
        {
            std::cerr << "UringManager: Submission queue is full!" << std::endl;
            return;
        }

        // 准备一个写操作
        io_uring_prep_write(sqe, fd, buffer.data(), buffer.size(), -1);

        CompletionCallback completion_cb = [cb = std::move(callback)](const io_uring_cqe* cqe)
        {
            cb(cqe->res);
        };

        submit_request(sqe, std::move(completion_cb));
    }

    void UringManager::run()
    {
        m_is_running.store(true);
        std::cout << "UringManager: Event loop started." << std::endl;

        while (m_is_running.load())
        {
            // 提交所有在队列中的SQE，并至少等待一个CQE返回。
            // 如果没有CQE，它会阻塞。
            io_uring_submit_and_wait(&m_ring, 1);

            struct io_uring_cqe* cqe;
            unsigned head;
            unsigned cqe_count = 0;

            // 遍历所有已完成的事件
            io_uring_for_each_cqe(&m_ring, head, cqe)
            {
                cqe_count++;
                uint64_t request_id = cqe->user_data;
                CompletionCallback cb;

                // ---- 关键的线程安全部分 ----
                {
                    std::lock_guard<std::mutex> lock(m_request_mutex);
                    auto it = m_active_requests.find(request_id);
                    if (it != m_active_requests.end())
                    {
                        cb = std::move(it->second); // 移动出回调
                        m_active_requests.erase(it); // 从map中移除
                    }
                } // 锁在这里释放

                if (cb)
                {
                    // 在锁外执行回调，防止回调函数中再次调用UringManager导致死锁
                    cb(cqe);
                }
                else
                {
                    std::cerr << "UringManager: Warning! Could not find callback for request ID " << request_id <<
                        std::endl;
                }
            }

            // 标记所有已处理的CQE为“已看到”
            if (cqe_count > 0)
            {
                io_uring_cq_advance(&m_ring, cqe_count);
            }
        }
        std::cout << "UringManager: Event loop stopped." << std::endl;
    }

    void UringManager::stop()
    {
        // 设置标志，让run()循环在下一次迭代时退出
        m_is_running.store(false);
        // 这里可以考虑注入一个no-op的io_uring操作来唤醒阻塞的run()循环，但对于简单场景，等待下一次事件也可以。
    }


    // --- Private Methods ---

    bool UringManager::initialize(unsigned int queue_depth)
    {
        if (io_uring_queue_init(queue_depth, &m_ring, 0) < 0)
        {
            perror("io_uring_queue_init");
            return false;
        }
        return true;
    }

    void UringManager::submit_request(struct io_uring_sqe* sqe, CompletionCallback callback)
    {
        uint64_t request_id;
        {
            std::lock_guard<std::mutex> lock(m_request_mutex);
            request_id = m_next_request_id++;
            m_active_requests[request_id] = std::move(callback);
        }

        // 将唯一的请求ID设置到user_data字段
        io_uring_sqe_set_data(sqe, (void*)request_id);
    }
}
