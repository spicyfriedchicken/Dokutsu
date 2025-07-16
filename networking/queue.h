#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

namespace net {

    template <typename T>
    class Queue {
    public:

        Queue = default();
        Queue(const Queue<T>&) = delete;
        virtual ~Queue() { clear(); }

         const T& front() const {
            std::lock_guard<std::mutex> lock(muxQueue_);
            return deQueue_.front();
        }
        const T& back() const {
            std::lock_guard<std::mutex> lock(muxQueue_);
            return deQueue_.back();
        }
        const T& pop_front() {
            std::lock_guard<std::mutex> lock(muxQueue_);
            return deQueue_.pop_front();
        }
        const T& pop_back() {
            std::lock_guard<std::mutex> lock(muxQueue_);
            return deQueue_.pop_back();
        }
        const
    private:
        std::mutex muxQueue_;
        std::deque<T> deQueue_;
        std::condition_variable cvBlock_;
        std::mutex muxBlocking_;

    }

}

#endif //QUEUE_H
