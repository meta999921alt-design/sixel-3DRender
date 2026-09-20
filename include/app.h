#pragma once
#include <fstream>
#include <utility>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>
#include "thread_pool.h"
#include "camera.h"
#include "input.h"
#include "physics.h"
#include "palette.h"
#include "sixel_encoder.h"

class Scene;

class App {
public:
    App();
    ~App();
    void run();

private:
    using clock = std::chrono::high_resolution_clock;
    static double ms(clock::time_point a, clock::time_point b);

    void render(const Scene& scene);
    void present(const std::string& frame);

    static std::pair<int, int> pickResolution();
    static std::string logPath();

    unsigned threads_;
    ThreadPool pool_;
    std::ofstream perfLog_;
    std::pair<int, int> res_;
    int w_, h_;
    Camera camera_;
    Input input_;
    World world_;
    Palette palette_;
    std::unique_ptr<Encoder> encoder_;
    std::vector<std::vector<uint8_t>> idx_;
};
