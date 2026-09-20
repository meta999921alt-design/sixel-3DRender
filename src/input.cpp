#include "input.h"
#include <iostream>
#include <string>
#include <vector>

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#endif

Input::Input() {
#if defined(_WIN32)
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hIn, &savedMode_);
    DWORD mode = (savedMode_ & ~ENABLE_QUICK_EDIT_MODE) | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(hIn, mode);
#elif defined(__linux__) || defined(__APPLE__)
    tcgetattr(STDIN_FILENO, &savedTerm_);
    termios raw = savedTerm_;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
    std::cout << "\x1b[?1003h\x1b[?1006h";
    std::cout.flush();
#endif
}

Input::~Input() {
#if defined(_WIN32)
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), savedMode_);
#elif defined(__linux__) || defined(__APPLE__)
    std::cout << "\x1b[?1003l\x1b[?1006l";
    std::cout.flush();
    tcsetattr(STDIN_FILENO, TCSANOW, &savedTerm_);
#endif
}

Input::State Input::poll() {
    State s;
#if defined(_WIN32)
    pollWin(s);
#elif defined(__linux__) || defined(__APPLE__)
    pollPosix(s);
    decay();
#endif
    s.fwd = (w_ ? 1.0 : 0.0) - (s_key_ ? 1.0 : 0.0);
    s.right = (d_ ? 1.0 : 0.0) - (a_ ? 1.0 : 0.0);
    return s;
}

#if defined(_WIN32)
void Input::pollWin(State& s) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD n = 0;
    GetNumberOfConsoleInputEvents(hIn, &n);
    if (!n) return;

    std::vector<INPUT_RECORD> recs(n);
    DWORD got = 0;
    if (!ReadConsoleInput(hIn, recs.data(), n, &got)) return;

    for (DWORD i = 0; i < got; i++) {
        auto& r = recs[i];
        if (r.EventType == KEY_EVENT) {
            bool down = r.Event.KeyEvent.bKeyDown != 0;
            switch (r.Event.KeyEvent.wVirtualKeyCode) {
                case 'W': w_ = down; break;
                case 'A': a_ = down; break;
                case 'S': s_key_ = down; break;
                case 'D': d_ = down; break;
                case VK_ESCAPE: if (down) s.quit = true; break;
            }
        } else if (r.EventType == MOUSE_EVENT) {
            auto& m = r.Event.MouseEvent;
            bool rmb = (m.dwButtonState & RIGHTMOST_BUTTON_PRESSED) != 0;
            if (m.dwEventFlags != MOUSE_MOVED) continue;
            if (rmb) {
                if (haveLast_) {
                    s.yawDelta += (m.dwMousePosition.X - lastX_) * kMouseSens;
                    s.pitchDelta -= (m.dwMousePosition.Y - lastY_) * kMouseSens;
                }
                lastX_ = m.dwMousePosition.X;
                lastY_ = m.dwMousePosition.Y;
                haveLast_ = true;
            } else {
                haveLast_ = false;
            }
        }
    }
}
#elif defined(__linux__) || defined(__APPLE__)
void Input::pollPosix(State& s) {
    char ch;
    while (read(STDIN_FILENO, &ch, 1) > 0) {
        if (ch == 0x1B) {
            if (!readMouseSeq(s)) s.quit = true;
            continue;
        }
        auto now = std::chrono::steady_clock::now();
        switch (ch) {
            case 'w': case 'W': w_ = true; seenW_ = now; break;
            case 'a': case 'A': a_ = true; seenA_ = now; break;
            case 's': case 'S': s_key_ = true; seenS_ = now; break;
            case 'd': case 'D': d_ = true; seenD_ = now; break;
        }
    }
}

bool Input::readMouseSeq(State& s) {
    char c1, c2;
    if (read(STDIN_FILENO, &c1, 1) <= 0 || c1 != '[') return false;
    if (read(STDIN_FILENO, &c2, 1) <= 0 || c2 != '<') return false;

    std::string digits;
    int button = 0, mx = 0, my = 0, field = 0;
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        if (c == ';' || c == 'M' || c == 'm') {
            int v = digits.empty() ? 0 : std::stoi(digits);
            if (field == 0) button = v;
            else if (field == 1) mx = v;
            else my = v;
            digits.clear();
            field++;
            if (c == 'M' || c == 'm') {
                bool rmb = (button & 3) == 2;
                bool down = rmb && (c == 'M');
                if (down) {
                    if (haveLast_) {
                        s.yawDelta += (mx - lastX_) * kMouseSens;
                        s.pitchDelta -= (my - lastY_) * kMouseSens;
                    }
                    lastX_ = mx;
                    lastY_ = my;
                    haveLast_ = true;
                } else {
                    haveLast_ = false;
                }
                return true;
            }
        } else if (c >= '0' && c <= '9') {
            digits += c;
        }
    }
    return true;
}

void Input::decay() {
    auto now = std::chrono::steady_clock::now();
    auto old = [&](auto t) { return std::chrono::duration<double>(now - t).count() > 0.2; };
    if (w_ && old(seenW_)) w_ = false;
    if (a_ && old(seenA_)) a_ = false;
    if (s_key_ && old(seenS_)) s_key_ = false;
    if (d_ && old(seenD_)) d_ = false;
}
#endif
