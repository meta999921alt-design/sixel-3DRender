#pragma once
#include <chrono>

#if defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#elif defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

class Input {
public:
    Input();
    ~Input();

    struct State {
        double yawDelta = 0, pitchDelta = 0;
        double fwd = 0, right = 0;
        bool quit = false;
    };

    State poll();

private:
#if defined(_WIN32)
    void pollWin(State& s);
#elif defined(__linux__) || defined(__APPLE__)
    void pollPosix(State& s);
    bool readMouseSeq(State& s);
    void decay();
#endif

    static constexpr double kMouseSens = 0.006;

    bool w_ = false, a_ = false, s_key_ = false, d_ = false;
    bool haveLast_ = false;
    int lastX_ = 0, lastY_ = 0;

#if defined(_WIN32)
    DWORD savedMode_ = 0;
#elif defined(__linux__) || defined(__APPLE__)
    termios savedTerm_{};
    std::chrono::steady_clock::time_point seenW_, seenA_, seenS_, seenD_;
#endif
};
