#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "thread_pool.h"

class Encoder {
public:
    Encoder(int width, int threads);

    std::string encode(const std::vector<std::vector<uint8_t>>& idx, int w, int h,
                        const std::string& palette, ThreadPool& pool);

private:
    struct ThreadState {
        std::vector<std::vector<uint8_t>> cols;
        std::vector<bool> seen;
        std::vector<int> used;
        std::string chunk;
        explicit ThreadState(int w);
    };

    static void encodeBand(const std::vector<std::vector<uint8_t>>& idx, int w, int start, int h, ThreadState& st);

    int w_;
    std::vector<ThreadState> states_;
};
