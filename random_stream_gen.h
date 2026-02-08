#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

using namespace std;

class RandomStreamGen {
public:
    struct SplitPlan {
        vector<size_t> boundaries;
    };

    RandomStreamGen(uint64_t seed = random_device{}(), size_t min_len = 1, size_t max_len = 30)
        : rng_(seed),
          min_len_(min_len),
          max_len_(max_len),
          len_dist_(static_cast<int>(min_len), static_cast<int>(max_len)),
          char_dist_(0, static_cast<int>(alphabet().size() - 1)) {}

    string next() {
        const size_t len = static_cast<size_t>(len_dist_(rng_));
        string s;
        s.reserve(len);
        const auto& chars = alphabet();
        for (size_t i = 0; i < len; ++i) {
            s.push_back(chars[static_cast<size_t>(char_dist_(rng_))]);
        }
        return s;
    }

    vector<string> next_batch(size_t count) {
        vector<string> batch;
        batch.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            batch.push_back(next());
        }
        return batch;
    }

    static SplitPlan split_by_percent(size_t total_items, double step_percent) {
        SplitPlan plan;
        if (total_items == 0 || step_percent <= 0.0) {
            return plan;
        }
        const double step = step_percent / 100.0;
        double cursor = step;
        while (cursor < 1.0 + 1e-12) {
            const size_t boundary = static_cast<size_t>(cursor * total_items + 0.5);
            if (!plan.boundaries.empty() && boundary <= plan.boundaries.back()) {
                cursor += step;
                continue;
            }
            plan.boundaries.push_back(boundary);
            cursor += step;
        }
        if (plan.boundaries.empty() || plan.boundaries.back() != total_items) {
            plan.boundaries.push_back(total_items);
        }
        return plan;
    }

private:
    static const string& alphabet() {
        static const string kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-";
        return kAlphabet;
    }

    mt19937_64 rng_;
    size_t min_len_;
    size_t max_len_;
    uniform_int_distribution<int> len_dist_;
    uniform_int_distribution<int> char_dist_;
};
