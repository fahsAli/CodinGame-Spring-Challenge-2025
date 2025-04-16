#pragma GCC optimize("Ofast,unroll-loops,inline")
#pragma GCC target("avx,avx2,fma,bmi,bmi2,popcnt,lzcnt")

#include <iostream>
#include <array>
#include <cstdint>

using namespace std;

constexpr uint64_t MOD = 1ULL << 30;

struct alignas(4) Board {
    uint32_t bits;

    [[gnu::always_inline]] inline uint8_t get_cell(int pos) const {
        return (bits >> (pos * 3)) & 7;
    }

    [[gnu::always_inline]] inline Board set_cell(int pos, uint8_t value) const {
        return Board{(bits & ~(7u << (pos * 3))) | (static_cast<uint32_t>(value & 7u) << (pos * 3))};
    }
};

constexpr array<array<int8_t, 4>, 9> NEIGHBORS = {{
    {1, 3, -1, -1}, {0, 2, 4, -1}, {1, 5, -1, -1},
    {0, 4, 6, -1}, {1, 3, 5, 7},   {2, 4, 8, -1},
    {3, 7, -1, -1}, {4, 6, 8, -1}, {5, 7, -1, -1}
}};

constexpr array<int8_t, 9> NEIGHBOR_COUNT = {2, 3, 2, 3, 4, 3, 2, 3, 2};

struct CaptureSet {
    uint8_t count;
    array<array<int8_t, 4>, 11> combos;
    array<uint8_t, 11> size;
};

constexpr array<CaptureSet, 9> CAPTURES = [] {
    array<CaptureSet, 9> table{};
    for (int pos = 0; pos < 9; ++pos) {
        auto& t = table[pos];
        int nc = NEIGHBOR_COUNT[pos];
        for (int i = 0; i < nc; i++) {
            for (int j = i + 1; j < nc; j++) {
                t.combos[t.count] = {{NEIGHBORS[pos][i], NEIGHBORS[pos][j], -1, -1}};
                t.size[t.count++] = 2;
            }
        }
        if (nc >= 3) {
            for (int i = 0; i < nc; i++) {
                for (int j = i + 1; j < nc; j++) {
                    for (int k = j + 1; k < nc; k++) {
                        t.combos[t.count] = {{NEIGHBORS[pos][i], NEIGHBORS[pos][j], NEIGHBORS[pos][k], -1}};
                        t.size[t.count++] = 3;
                    }
                }
            }
        }
        if (nc == 4) {
            t.combos[t.count] = {{NEIGHBORS[pos][0], NEIGHBORS[pos][1], NEIGHBORS[pos][2], NEIGHBORS[pos][3]}};
            t.size[t.count++] = 4;
        }
    }
    return table;
}();

struct alignas(4) MemoEntry {
    uint32_t board_bits;
    uint32_t result;
    uint16_t depth_valid;

    [[gnu::always_inline]] inline uint8_t depth() const { return depth_valid & 0xFF; }
    [[gnu::always_inline]] inline bool valid() const { return (depth_valid >> 8) & 1; }
    [[gnu::always_inline]] inline void set(uint8_t d, bool v) { depth_valid = d | (v << 8); }
};

[[gnu::always_inline]] inline size_t table_size(int depth) {
    return 1ULL << min((depth + 4) / 5 * 2 + 12, 21);
}

MemoEntry* memoTable;
size_t tableSize;
uint32_t tableMask;

[[gnu::always_inline]] inline uint32_t calculate_hash(uint32_t bits, uint8_t depth) {
    uint64_t h = bits * 0x9E3779B97F4A7C15ULL; 
    h ^= (h >> 32);
    h ^= depth;
    return h & tableMask;
}

[[gnu::always_inline]] inline uint16_t get_empty_mask(const Board& b) {
    uint16_t mask = 0;
    for (int i = 0; i < 9; ++i)
        if (b.get_cell(i) == 0) mask |= (1 << i);
    return mask;
}

[[gnu::always_inline]] inline uint32_t calc_board_value(const Board& b) {
    uint32_t v = 0;
    for (int i = 0; i < 9; ++i)
        v = v * 10 + b.get_cell(i);
    return v & (MOD - 1);
}

[[gnu::always_inline]] inline bool valid_capture(const Board& b, const auto& c, int size, uint8_t& sum) {
    sum = 0;
    for (int i = 0; i < size; ++i) {
        uint8_t val = b.get_cell(c[i]);
        if (val == 0) return false;
        sum += val;
    }
    return sum <= 6;
}

uint32_t simulate(const Board& b, int max_depth, int d) {
    if (d == max_depth) return calc_board_value(b);

    uint16_t mask = get_empty_mask(b);
    if (!mask) return calc_board_value(b);

    uint32_t h = calculate_hash(b.bits, d);
    MemoEntry& e = memoTable[h];
    if (e.valid() && e.board_bits == b.bits && e.depth() == d) return e.result;

    uint32_t res = 0;

    while (mask) {
        uint16_t lsb = mask & -mask;
        int pos = __builtin_ctz(lsb);
        mask ^= lsb;

        bool captured = false;
        for (int i = 0; i < CAPTURES[pos].count; ++i) {
            auto& combo = CAPTURES[pos].combos[i];
            uint8_t sum;
            if (valid_capture(b, combo, CAPTURES[pos].size[i], sum)) {
                Board nb = b;
                for (int j = 0; j < CAPTURES[pos].size[i]; ++j)
                    nb = nb.set_cell(combo[j], 0);
                nb = nb.set_cell(pos, sum);
                res = (res + simulate(nb, max_depth, d + 1)) & (MOD - 1);
                captured = true;
            }
        }

        if (!captured) {
            res = (res + simulate(b.set_cell(pos, 1), max_depth, d + 1)) & (MOD - 1);
        }
    }

    e.board_bits = b.bits;
    e.set(d, true);
    e.result = res;
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    int depth;
    cin >> depth;
    Board b{0};

    for (int i = 0; i < 9; ++i) {
        int x;
        cin >> x;
        b = b.set_cell(i, x);
    }

    tableSize = table_size(depth);
    tableMask = tableSize - 1;
    memoTable = new MemoEntry[tableSize]();

    uint32_t result = simulate(b, depth, 0);
    cout << result << '\n';

    delete[] memoTable;
    return 0;
}
