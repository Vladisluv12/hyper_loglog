#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

using namespace std;

class HyperLogLog {
public:
    explicit HyperLogLog(uint8_t p) : p_(p), m_(static_cast<size_t>(1u << p)), registers_(m_, 0) {}

    void add(uint64_t hash) {
        const uint64_t idx = hash >> (64u - p_);
        uint64_t w = hash << p_;
        uint8_t rank = static_cast<uint8_t>(leading_zeros_64(w) + 1u);
        const uint8_t max_rank = static_cast<uint8_t>(64u - p_ + 1u);
        if (rank > max_rank) {
            rank = max_rank;
        }
        uint8_t& reg = registers_[static_cast<size_t>(idx)];
        if (rank > reg) {
            reg = rank;
        }
    }

    double estimate() const {
        const double m = static_cast<double>(m_);
        double inv_sum = 0.0;
        size_t zeros = 0;
        for (uint8_t v : registers_) {
            inv_sum += ldexp(1.0, -static_cast<int>(v));
            if (v == 0) {
                ++zeros;
            }
        }
        const double alpha = 0.7213 / (1.0 + 1.079 / static_cast<double>(m));
        double e = alpha * m * m / inv_sum;

        if (e <= 2.5 * m && zeros > 0) {
            e = m * log(m / static_cast<double>(zeros));
        } else if (e > (1.0 / 30.0) * 18446744073709551616.0) {
            e = -18446744073709551616.0 * log(1.0 - (e / 18446744073709551616.0));
        }
        return e;
    }

    size_t register_count() const { return m_; }
    uint8_t precision() const { return p_; }

private:
    static uint8_t leading_zeros_64(uint64_t x) {
        if (x == 0) {
            return 64;
        }
        return static_cast<uint8_t>(__builtin_clz(x));
    }

    uint8_t p_;
    size_t m_;
    vector<uint8_t> registers_;
};
