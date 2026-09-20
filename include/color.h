#pragma once
#include <algorithm>

struct Color {
    double r = 0, g = 0, b = 0;

    Color() = default;
    Color(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}

    Color operator+(const Color& o) const { return { r + o.r, g + o.g, b + o.b }; }
    Color operator*(double f) const { return { r * f, g * f, b * f }; }

    Color clamped() const {
        return { std::clamp(r, 0.0, 255.0), std::clamp(g, 0.0, 255.0), std::clamp(b, 0.0, 255.0) };
    }
};
