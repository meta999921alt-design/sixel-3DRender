#pragma once
#include <cstdint>
#include <vector>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    struct InputState {
        double yawDelta = 0, pitchDelta = 0;
        double fwd = 0, right = 0;
        bool reset = false;
        bool quit = false;
    };

    InputState poll();
    void present(const std::vector<uint32_t>& pixels);
    void setTitle(const std::string& title);

private:
    static LRESULT CALLBACK wndProcThunk(HWND, UINT, WPARAM, LPARAM);
    LRESULT handleMessage(HWND, UINT, WPARAM, LPARAM);

    static constexpr double kMouseSens = 0.004;

    HWND hwnd_ = nullptr;
    HDC memDC_ = nullptr;
    HBITMAP bitmap_ = nullptr;
    HBITMAP oldBitmap_ = nullptr;
    void* bits_ = nullptr;
    int width_, height_;
    bool shouldQuit_ = false;

    bool keyW_ = false, keyA_ = false, keyS_ = false, keyD_ = false;
    bool resetPressed_ = false;
    bool rightDown_ = false;
    bool haveLastMouse_ = false;
    int lastMouseX_ = 0, lastMouseY_ = 0;
    double pendingYaw_ = 0, pendingPitch_ = 0;
};
