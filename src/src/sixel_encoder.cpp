#include "sixel_encoder.h"
#include <algorithm>

Encoder::ThreadState::ThreadState(int w) : cols(256, std::vector<uint8_t>(w, 0)), seen(256, false) {
    used.reserve(256);
}

Encoder::Encoder(int width, int threads) : w_(width) {
    for (int i = 0; i < threads; i++) states_.emplace_back(width);
}

std::string Encoder::encode(const std::vector<std::vector<uint8_t>>& idx, int w, int h,
                             const std::string& palette, ThreadPool& pool) {
    int bandH = 6;
    int bands = (h + bandH - 1) / bandH;
    unsigned n = (unsigned)states_.size();

    std::vector<int> range(n + 1);
    for (unsigned i = 0; i <= n; i++) range[i] = (int)((long long)bands * i / n);

    pool.parallelFor([&](unsigned t) {
        auto& st = states_[t];
        st.chunk.clear();
        for (int band = range[t]; band < range[t + 1]; band++) {
            int start = band * bandH;
            int h2 = std::min(bandH, h - start);
            encodeBand(idx, w, start, h2, st);
        }
    });

    std::string out;
    size_t len = palette.size() + 64;
    for (auto& st : states_) len += st.chunk.size();
    out.reserve(len);

    out += "\x1bPq\"1;1;" + std::to_string(w) + ";" + std::to_string(h);
    out += palette;
    for (auto& st : states_) out += st.chunk;
    out += "\x1b\\";
    return out;
}

void Encoder::encodeBand(const std::vector<std::vector<uint8_t>>& idx, int w, int start, int h, ThreadState& st) {
    st.used.clear();
    for (int x = 0; x < w; x++)
        for (int dy = 0; dy < h; dy++) {
            uint8_t reg = idx[start + dy][x];
            if (!st.seen[reg]) { st.seen[reg] = true; st.used.push_back(reg); }
            st.cols[reg][x] |= (uint8_t)(1u << dy);
        }

    bool first = true;
    for (int reg : st.used) {
        if (!first) st.chunk += "$";
        first = false;
        st.chunk += "#" + std::to_string(reg);

        auto& col = st.cols[reg];
        int x = 0;
        while (x < w) {
            uint8_t val = col[x];
            int run = 1;
            while (x + run < w && col[x + run] == val) run++;
            char ch = (char)(0x3F + val);
            if (run > 3) st.chunk += "!" + std::to_string(run) + ch;
            else st.chunk.append((size_t)run, ch);
            x += run;
        }
        std::fill(col.begin(), col.end(), 0);
        st.seen[reg] = false;
    }
    st.chunk += "-";
}
