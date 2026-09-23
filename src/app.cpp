#include "app.h"
#include "scene.h"
#include "constants.h"
#include <algorithm>
#include <thread>
#include <cmath>

namespace {
PhysicsEngine buildPhysics() {
    PhysicsEngine engine;
    engine.addPlane({ Vector3(0, kFloorY, 0), Vector3(0, 1, 0), 0.25, 0.5 });
    engine.addPlane({ kRampBase, kRampNormal, 0.15, 0.15 });

    Vector3 spawn = kRampBase + kRampSlopeDir * (kRampLength * 0.85) + Vector3(0, 0.8, 0);
    engine.addBody(RigidBody(spawn, 0.4, 1.0, Color(220, 70, 70), 0.35, 0.2));
    return engine;
}
}

App::App()
    : threads_(std::max(1u, std::thread::hardware_concurrency())),
      pool_(threads_),
      w_(kWindowWidth),
      h_(kWindowHeight),
      window_(w_, h_, "Physics Demo"),
      camera_(w_, h_),
      physics_(buildPhysics()),
      pixels_((size_t)w_ * h_, 0)
{
}

void App::run() {
    double t = 0;
    int frameNo = 0;
    const std::chrono::microseconds targetDt(10000);
    auto prevTime = clock::now();
    auto fpsTimer = clock::now();
    double renderMsAccum = 0, presentMsAccum = 0;

    while (true) {
        auto start = clock::now();
        double dt = std::clamp(std::chrono::duration<double>(start - prevTime).count(), 0.0, 0.1);
        prevTime = start;

        auto in = window_.poll();
        if (in.quit) break;
        camera_.rotate(in.yawDelta, in.pitchDelta);
        camera_.move(in.fwd * 3.0 * dt, in.right * 3.0 * dt);
        if (in.reset) physics_.reset();

        physics_.step(dt);

        Scene scene = buildScene(t, physics_);
        render(scene);
        auto afterRender = clock::now();

        window_.present(pixels_);
        auto afterPresent = clock::now();

        renderMsAccum += ms(start, afterRender);
        presentMsAccum += ms(afterRender, afterPresent);

        t += 0.03;
        frameNo++;
        if (frameNo % 30 == 0) {
            double secs = std::chrono::duration<double>(clock::now() - fpsTimer).count();
            fpsTimer = clock::now();
            double fps = secs > 0 ? 30.0 / secs : 0.0;
            double avgRender = renderMsAccum / 30.0;
            double avgPresent = presentMsAccum / 30.0;
            renderMsAccum = 0;
            presentMsAccum = 0;
            window_.setTitle(
                "Physics Demo - " + std::to_string((int)fps) + " fps | render " +
                std::to_string((int)avgRender) + "ms present " + std::to_string((int)avgPresent) +
                "ms | R to reset ball"
            );
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - start);
        if (elapsed < targetDt) std::this_thread::sleep_for(targetDt - elapsed);
    }
}

double App::ms(clock::time_point a, clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

void App::render(const Scene& scene) {
    unsigned n = threads_;
    std::vector<int> rowStart(n + 1);
    for (unsigned i = 0; i <= n; i++) rowStart[i] = (int)((long long)h_ * i / n);

    pool_.parallelFor([&](unsigned t) {
        for (int y = rowStart[t]; y < rowStart[t + 1]; y++)
            for (int x = 0; x < w_; x++) {
                Color c = scene.trace(camera_.rayForPixel(x, y), 0).clamped();
                uint32_t r = (uint32_t)std::lround(c.r);
                uint32_t g = (uint32_t)std::lround(c.g);
                uint32_t b = (uint32_t)std::lround(c.b);
                pixels_[(size_t)y * w_ + x] = (r << 16) | (g << 8) | b;
            }
    });
}
