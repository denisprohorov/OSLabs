#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class BufferedChannel {
private:
    std::queue<T> queue;
    const size_t size;
    std::mutex mutex;
    std::condition_variable condSend;
    std::condition_variable condRecv;
    bool isOpen;

public:
    explicit BufferedChannel(int size) : size(size), isOpen(true) {}

    void Send(T value){
        std::unique_lock<std::mutex> lock(this->mutex);
        condSend.wait(lock, [this](){return (queue.size() < this->size || !isOpen);});

        if(!isOpen) throw(std::runtime_error("Channel is closed, sorry"));
        queue.push(value);
        lock.unlock();
        condRecv.notify_one();
    }

    std::pair<T, bool> Recv() {
        std::unique_lock<std::mutex> lock(this->mutex);
        condRecv.wait(lock, [this](){return (!this->queue.empty() || !isOpen);});
        if(queue.empty()){
            std::pair<T, bool> pair;
            pair.second = false;
            lock.unlock();
            return pair;
        }
        //queue is not empty
        std::pair<T, bool> pair(queue.front(), true);
        queue.pop();
        lock.unlock();
        condSend.notify_one();
        return pair;
    }

    void Close() {
        std::unique_lock<std::mutex> lock(this->mutex);
        isOpen = false;
        lock.unlock();
        condRecv.notify_all();
        condSend.notify_all();
    }
};

#endif // BUFFERED_CHANNEL_H_