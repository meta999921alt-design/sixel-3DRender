#include "app.h"
#include "scene.h"
#include "platform.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <filesystem>

App::App()
    : threads_(std::max(1u, std::thread::hardware_concurrency())),
      pool_(threads_),
      perfLog_(logPath(), std::ios::app),
      res_(pickResolution()),
      w_(res_.first),
      h_(res_.second),
      camera_(w_, h_),
      encoder_(std::make_unique<Encoder>(w_, (int)threads_)),
      idx_(h_, std::vector<uint8_t>(w_, 0))
{
    std::cout << "\x1b[2J\x1b[?25l";
    std::cerr << "[DIAG] perf log path: " << logPath() << "\n";
    if (!perfLog_.is_open()) std::cerr << "[DIAG] failed to open perf.log\n";
    perfLog_ << "run start: " << w_ << "x" << h_ << ", threads=" << threads_ << "\n";
    perfLog_.flush();
    std::cerr << "[DIAG] WASD to move, right-drag mouse to look, ESC to quit\n";
}

App::~App() { std::cout << "\x1b[?25h"; }

void App::run() {
    double t = 0;
    int frameNo = 0;
    const std::chrono::microseconds targetDt(10000);
    auto prevTime = clock::now();

    while (true) {
        auto start = clock::now();
        double dt = std::clamp(std::chrono::duration<double>(start - prevTime).count(), 0.0, 0.1);
        prevTime = start;

        auto in = input_.poll();
        if (in.quit) break;
        camera_.rotate(in.yawDelta, in.pitchDelta);
        camera_.move(in.fwd * 3.0 * dt, in.right * 3.0 * dt);

        world_.update(dt);

        Scene scene = buildScene(t, world_);
        render(scene);
        auto tTrace = clock::now();

        std::string frame = encoder_->encode(idx_, w_, h_, palette_.declaration(), pool_);
        auto tEncode = clock::now();

        present(frame);
        auto tIo = clock::now();

        frameNo++;
        if (frameNo == 1 || frameNo % 30 == 0) {
            double a = ms(start, tTrace), b = ms(tTrace, tEncode), c = ms(tEncode, tIo);
            double total = a + b + c;
            perfLog_ << "frame " << frameNo << " | trace=" << a << "ms encode=" << b << "ms io=" << c
                     << "ms | total=" << total << "ms (~" << (1000.0 / total) << " fps)\n";
            perfLog_.flush();
            if (frameNo == 1) std::cerr << "[DIAG] first frame done: " << total << "ms\n";
        }

        t += 0.03;
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(tIo - start);
        if (elapsed < targetDt) std::this_thread::sleep_for(targetDt - elapsed);
    }
}

double App::ms(clock::time_point a, clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

void App::render(const Scene& scene) {
    unsigned n = threads_;
    pool_.parallelFor([&](unsigned t) {
        for (int y = t; y < h_; y += n)
            for (int x = 0; x < w_; x++) {
                Color c = scene.trace(camera_.rayForPixel(x, y), 0);
                idx_[y][x] = palette_.quantize(c, x, y);
            }
    });
}

void App::present(const std::string& frame) {
    std::cout << "\x1b[H";
    std::cout.write(frame.c_str(), (std::streamsize)frame.size());
    std::cout << "\x1b[0J";
    std::cout.flush();
}

std::pair<int, int> App::pickResolution() {
    consoleInit();
    int w, h;
    guessResolution(1920, 1080, w, h);
    return { w, h };
}

std::string App::logPath() {
    try { return std::filesystem::absolute("perf.log").string(); }
    catch (...) { return "perf.log"; }
}
