#include "window.h"
#include <windowsx.h>
#include <cstring>

namespace {
const wchar_t* kClassName = L"RaytracerWindowClass";
}

Window::Window(int width, int height, const std::string& title) : width_(width), height_(height) {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSW wc = {};
    wc.lpfnWndProc = &Window::wndProcThunk;
    wc.hInstance = hInstance;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.style = CS_OWNDC;
    RegisterClassW(&wc);

    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME) & ~WS_MAXIMIZEBOX;
    RECT rect = { 0, 0, width_, height_ };
    AdjustWindowRect(&rect, style, FALSE);

    std::wstring wtitle(title.begin(), title.end());
    hwnd_ = CreateWindowExW(0, kClassName, wtitle.c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, this);

    ShowWindow(hwnd_, SW_SHOW);

    HDC windowDC = GetDC(hwnd_);
    memDC_ = CreateCompatibleDC(windowDC);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width_;
    bmi.bmiHeader.biHeight = -height_; // 음수 = top-down (우리 버퍼가 위에서 아래로 채워지므로)
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    bitmap_ = CreateDIBSection(memDC_, &bmi, DIB_RGB_COLORS, &bits_, nullptr, 0);
    oldBitmap_ = (HBITMAP)SelectObject(memDC_, bitmap_);

    ReleaseDC(hwnd_, windowDC);
}

Window::~Window() {
    if (memDC_) {
        SelectObject(memDC_, oldBitmap_);
        DeleteObject(bitmap_);
        DeleteDC(memDC_);
    }
    if (hwnd_) DestroyWindow(hwnd_);
}

Window::InputState Window::poll() {
    pendingYaw_ = 0;
    pendingPitch_ = 0;
    resetPressed_ = false;

    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) { shouldQuit_ = true; break; }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    InputState s;
    s.quit = shouldQuit_;
    s.yawDelta = pendingYaw_;
    s.pitchDelta = pendingPitch_;
    s.reset = resetPressed_;
    s.fwd = (keyW_ ? 1.0 : 0.0) - (keyS_ ? 1.0 : 0.0);
    s.right = (keyD_ ? 1.0 : 0.0) - (keyA_ ? 1.0 : 0.0);
    return s;
}

void Window::present(const std::vector<uint32_t>& pixels) {
    if (bits_) std::memcpy(bits_, pixels.data(), pixels.size() * sizeof(uint32_t));
    HDC windowDC = GetDC(hwnd_);
    BitBlt(windowDC, 0, 0, width_, height_, memDC_, 0, 0, SRCCOPY);
    ReleaseDC(hwnd_, windowDC);
}

void Window::setTitle(const std::string& title) {
    std::wstring wtitle(title.begin(), title.end());
    SetWindowTextW(hwnd_, wtitle.c_str());
}

LRESULT CALLBACK Window::wndProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (self) return self->handleMessage(hwnd, msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Window::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            shouldQuit_ = true;
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
        case WM_KEYUP: {
            bool down = (msg == WM_KEYDOWN);
            switch (wParam) {
                case 'W': keyW_ = down; break;
                case 'A': keyA_ = down; break;
                case 'S': keyS_ = down; break;
                case 'D': keyD_ = down; break;
                case 'R': if (down) resetPressed_ = true; break;
                case VK_ESCAPE: if (down) shouldQuit_ = true; break;
            }
            return 0;
        }
        case WM_RBUTTONDOWN:
            rightDown_ = true;
            haveLastMouse_ = false;
            SetCapture(hwnd);
            return 0;
        case WM_RBUTTONUP:
            rightDown_ = false;
            ReleaseCapture();
            return 0;
        case WM_MOUSEMOVE: {
            if (rightDown_) {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                if (haveLastMouse_) {
                    pendingYaw_ += (x - lastMouseX_) * kMouseSens;
                    pendingPitch_ -= (y - lastMouseY_) * kMouseSens;
                }
                lastMouseX_ = x;
                lastMouseY_ = y;
                haveLastMouse_ = true;
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}
