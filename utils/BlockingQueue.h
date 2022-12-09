#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>

/* thread safe queue, operation dequeue will block if queue is empty
 * so you might check empty before dequeue, if you don't want to block
 *
 * notice: use smart pointer as T may have a better performance */
template <typename T>
class BlockingQueue {
public:
    BlockingQueue() = default;
    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue(BlockingQueue&&) = delete;
    ~BlockingQueue() = default;
    BlockingQueue& operator=(const BlockingQueue&) = delete;
    BlockingQueue& operator=(BlockingQueue&&) = delete;

    bool empty() {
        std::lock_guard<std::mutex> lock(queue_lock_);
        return queue_.empty();
    }
    int size() {
        std::lock_guard<std::mutex> lock(queue_lock_);
        return queue_.size();
    }
    void clear() {
        std::lock_guard<std::mutex> lock(queue_lock_);
        while(!queue_.empty()){
            queue_.pop();
        }
    }
    void enqueue(T& obj) {
        std::lock_guard<std::mutex> lock(queue_lock_);
        queue_.emplace(obj);
        cond_.notify_one();
    }
    T dequeue() {
        std::lock_guard<std::mutex> lock(queue_lock_);
        cond_.wait(queue_lock_, [this](){
            return !queue_.empty();
        });
        T obj = queue_.front();
        queue_.pop();
        return obj;
    }

private:
    std::queue<T> queue_;
    std::mutex queue_lock_;
    std::condition_variable cond_;
};