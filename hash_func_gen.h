#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>
#include <iterator>

using namespace std;

class HashFuncGen {
public:
    HashFuncGen(uint64_t seed = random_device{}()) : seed_(seed) {}

    uint64_t operator()(const string& s) const {
        uint64_t h1 = seed_ ^ 0xcbf29ce484222325ull;
        uint64_t h2 = seed_ ^ 0x100000001b3ull;
        for (unsigned char c : s) {
            h1 ^= static_cast<uint64_t>(c);
            h1 *= 1099511628211ull;
            h2 ^= static_cast<uint64_t>(c);
            h2 = (h2 << 13) | (h2 >> 51);
            h2 = h2 * 5 + 0x7b7d159c;
        }
        uint64_t len_mix = static_cast<uint64_t>(s.length());
        len_mix = (len_mix * 0xcc9e2d51) ^ (len_mix >> 32);
        h1 ^= len_mix;
        h2 ^= len_mix;
        
        uint64_t combined = h1 ^ h2 ^ (static_cast<uint64_t>(s.length()) << 32);
        return finalize64(combined);
    }
    
    static double chi_square_uniformity(const vector<uint64_t>& hashes, size_t buckets = 256u) {
        if (hashes.empty() || buckets == 0u) {
            return 0.0;
        }
        vector<size_t> counts(buckets, 0);
        for (uint64_t h : hashes) {
            counts[static_cast<size_t>(h % buckets)]++;
        }
        const double expected = static_cast<double>(hashes.size()) / buckets;
        if (expected == 0.0) {
            return 0.0;
        }
        double chi = 0.0;
        for (size_t c : counts) {
            const double diff = static_cast<double>(c) - expected;
            chi += (diff * diff) / expected;
        }
        return chi;
    }

private:
    static uint64_t finalize64(uint64_t x) {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdull;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ull;
        x ^= x >> 33;
        return x;
    }

    uint64_t seed_;
};