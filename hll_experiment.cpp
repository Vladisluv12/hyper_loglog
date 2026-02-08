#include <bits/stdc++.h>

#include "hash_func_gen.h"
#include "hyperloglog.h"
#include "random_stream_gen.h"

using namespace std;

struct StepResult {
    size_t items = 0;
    size_t exact = 0;
    double estimate = 0.0;
};

static vector<StepResult> run_stream(uint64_t seed, size_t total_items, double step_percent, uint8_t p) {
    RandomStreamGen stream(seed);
    HashFuncGen hash_gen(seed + 99991);
    HyperLogLog hll(p);
    unordered_set<string> exact_set;
    exact_set.reserve(total_items / 2);

    RandomStreamGen::SplitPlan plan = RandomStreamGen::split_by_percent(total_items, step_percent);
    vector<StepResult> results(plan.boundaries.size());

    size_t boundary_idx = 0;
    for (size_t i = 1; i <= total_items; ++i) {
        string s = stream.next();
        exact_set.insert(s);
        const uint64_t h = hash_gen(s);
        hll.add(h);

        if (boundary_idx < plan.boundaries.size() && i == plan.boundaries[boundary_idx]) {
            StepResult r;
            r.items = i;
            r.exact = exact_set.size();
            r.estimate = hll.estimate();
            results[boundary_idx] = r;
            ++boundary_idx;
        }
    }

    return results;
}

static void write_graph1_csv(const string& path, const vector<vector<StepResult>>& all_results) {
    ofstream out(path);
    out << "stream_id,step_index,items,exact,estimate\n";
    for (size_t s = 0; s < all_results.size(); ++s) {
        const auto& steps = all_results[s];
        for (size_t i = 0; i < steps.size(); ++i) {
            out << s << "," << i << "," << steps[i].items << "," << steps[i].exact
                << "," << fixed << setprecision(3) << steps[i].estimate << "\n";
        }
    }
}

static void write_graph2_csv(const string& path, const vector<vector<StepResult>>& all_results) {
    if (all_results.empty()) {
        return;
    }
    const size_t steps = all_results.front().size();
    ofstream out(path);
    out << "step_index,items,mean_estimate,stddev_estimate\n";
    for (size_t i = 0; i < steps; ++i) {
        const size_t items = all_results.front()[i].items;
        double mean = 0.0;
        for (const auto& stream : all_results) {
            mean += stream[i].estimate;
        }
        mean /= static_cast<double>(all_results.size());

        double var = 0.0;
        for (const auto& stream : all_results) {
            const double diff = stream[i].estimate - mean;
            var += diff * diff;
        }
        var /= static_cast<double>(all_results.size());
        const double stddev = sqrt(var);

        out << i << "," << items << "," << fixed << setprecision(3) << mean
            << "," << stddev << "\n";
    }
}

static void print_bucket_balance(uint64_t seed, size_t total_items, const vector<uint8_t>& p_candidates) {
    RandomStreamGen stream(seed);
    HashFuncGen hash_gen(seed + 123);

    vector<uint64_t> hashes;
    hashes.reserve(total_items);
    for (size_t i = 0; i < total_items; ++i) {
        hashes.push_back(hash_gen(stream.next()));
    }

    cout << "Bucket balance check (min/mean/max counts):\n";
    for (uint8_t p : p_candidates) {
        const size_t m = static_cast<size_t>(1u << p);
        vector<size_t> counts(m, 0);
        for (uint64_t h : hashes) {
            const size_t idx = static_cast<size_t>(h >> (64u - p));
            counts[idx]++;
        }
        size_t min_c = counts[0];
        size_t max_c = counts[0];
        size_t sum = 0;
        for (size_t c : counts) {
            sum += c;
            if (c < min_c) {
                min_c = c;
            }
            if (c > max_c) {
                max_c = c;
            }
        }
        const double mean = static_cast<double>(sum) / static_cast<double>(m);
        cout << "  p=" << static_cast<int>(p) << " m=" << m << " min=" << min_c << " mean=" << fixed << setprecision(2) << mean << " max=" << max_c << "\n";
    }
}

int main() {
    const size_t total_items = 50000;
    const double step_percent = 10.0;
    const uint8_t p = 10;
    const size_t streams = 5;

    print_bucket_balance(7777, 20000, {8, 10, 12});

    vector<vector<StepResult>> all_results;
    all_results.reserve(streams);
    for (size_t i = 0; i < streams; ++i) {
        all_results.push_back(run_stream(1000 + i * 97, total_items, step_percent, p));
    }

    write_graph1_csv("graph1_data.csv", all_results);
    write_graph2_csv("graph2_stats.csv", all_results);

    cout << "Wrote graph1_data.csv and graph2_stats.csv\n";
    return 0;
}
