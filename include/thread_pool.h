#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(unsigned n);
    ~ThreadPool();

    unsigned size() const { return (unsigned)workers_.size(); }
    void parallelFor(const std::function<void(unsigned)>& task);

private:
    void loop(unsigned idx);

    std::vector<std::thread> workers_;
    std::mutex mtx_, doneMtx_;
    std::condition_variable cv_, doneCv_;
    std::function<void(unsigned)> task_;
    unsigned long long gen_;
    size_t remaining_;
    bool stop_;
};
