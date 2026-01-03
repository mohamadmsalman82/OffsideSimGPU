#pragma once

#include <cstdint>
#include <random>

class RNG {
public:
    explicit RNG(std::uint64_t seed) : seed_(seed), engine_(static_cast<std::mt19937::result_type>(seed)) {}

    float uniform(float a, float b) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(engine_);
    }

    float normal(float mean, float stddev) {
        std::normal_distribution<float> dist(mean, stddev);
        return dist(engine_);
    }

    bool bernoulli(float p) {
        std::bernoulli_distribution dist(p);
        return dist(engine_);
    }

    int int_range(int lo, int hi) {
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(engine_);
    }

    std::uint64_t seed() const { return seed_; }

private:
    std::uint64_t seed_;
    std::mt19937 engine_;
};

