#pragma once
#include <string>
#include <cstdint>
#include "color.h"

class Palette {
public:
    static constexpr int kR = 8, kG = 8, kB = 4;

    Palette();

    uint8_t quantize(const Color& c, int x, int y) const;
    const std::string& declaration() const { return decl_; }

    static int pack(int r, int g, int b);

private:
    static const int kBayer[8][8];

    static int pct(int level, int levels);
    static int chan(double v, int levels, int x, int y);

    std::string decl_;
};
