#include "palette.h"
#include <cmath>
#include <algorithm>

const int Palette::kBayer[8][8] = {
    {0, 32, 8, 40, 2, 34, 10, 42}, {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44, 4, 36, 14, 46, 6, 38}, {60, 28, 52, 20, 62, 30, 54, 22},
    {3, 35, 11, 43, 1, 33, 9, 41}, {51, 19, 59, 27, 49, 17, 57, 25},
    {15, 47, 7, 39, 13, 45, 5, 37}, {63, 31, 55, 23, 61, 29, 53, 21}
};

Palette::Palette() {
    decl_.reserve(kR * kG * kB * 20);
    for (int r = 0; r < kR; r++)
        for (int g = 0; g < kG; g++)
            for (int b = 0; b < kB; b++) {
                int reg = pack(r, g, b);
                decl_ += "#" + std::to_string(reg) + ";2;" +
                    std::to_string(pct(r, kR)) + ";" + std::to_string(pct(g, kG)) + ";" + std::to_string(pct(b, kB));
            }
}

uint8_t Palette::quantize(const Color& c, int x, int y) const {
    return (uint8_t)pack(chan(c.r, kR, x, y), chan(c.g, kG, x, y), chan(c.b, kB, x, y));
}

int Palette::pack(int r, int g, int b) { return r * (kG * kB) + g * kB + b; }

int Palette::pct(int level, int levels) {
    return (int)std::lround(level * (255.0 / (levels - 1)) / 255.0 * 100.0);
}

int Palette::chan(double v, int levels, int x, int y) {
    double scaled = std::clamp(v, 0.0, 255.0) / 255.0 * (levels - 1);
    int lo = (int)std::floor(scaled);
    double frac = scaled - lo;
    double threshold = (kBayer[y & 7][x & 7] + 0.5) / 64.0;
    return std::clamp(frac > threshold ? lo + 1 : lo, 0, levels - 1);
}
