#include "thread_pool.h"

ThreadPool::ThreadPool(unsigned n) : gen_(0), remaining_(0), stop_(false) {
    for (unsigned i = 0; i < n; i++) workers_.emplace_back([this, i] { loop(i); });
}

ThreadPool::~ThreadPool() {
    { std::lock_guard<std::mutex> lk(mtx_); stop_ = true; }
    cv_.notify_all();
    for (auto& w : workers_) w.join();
}

void ThreadPool::parallelFor(const std::function<void(unsigned)>& task) {
    {
        std::lock_guard<std::mutex> lk(mtx_);
        task_ = task;
        remaining_ = workers_.size();
        gen_++;
    }
    cv_.notify_all();
    std::unique_lock<std::mutex> lk(doneMtx_);
    doneCv_.wait(lk, [this] { return remaining_ == 0; });
}

void ThreadPool::loop(unsigned idx) {
    unsigned long long seen = 0;
    while (true) {
        std::function<void(unsigned)> job;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_.wait(lk, [this, seen] { return stop_ || gen_ != seen; });
            if (stop_) return;
            seen = gen_;
            job = task_;
        }
        job(idx);
        {
            std::lock_guard<std::mutex> lk(doneMtx_);
            if (--remaining_ == 0) doneCv_.notify_one();
        }
    }
}
