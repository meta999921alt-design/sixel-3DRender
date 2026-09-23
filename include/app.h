#pragma once
#include <vector>
#include <cstdint>
#include <chrono>
#include <string>
#include "thread_pool.h"
#include "camera.h"
#include "physics.h"
#include "window.h"

class Scene;

class App {
public:
    App();
    void run();

private:
    using clock = std::chrono::high_resolution_clock;
    static double ms(clock::time_point a, clock::time_point b);

    void render(const Scene& scene);

    static constexpr int kWindowWidth = 1280;
    static constexpr int kWindowHeight = 720;

    unsigned threads_;
    ThreadPool pool_;
    int w_, h_;
    Window window_;
    Camera camera_;
    PhysicsEngine physics_;
    std::vector<uint32_t> pixels_;
};
