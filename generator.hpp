#pragma once

#include "rng.hpp"
#include "scenario.hpp"

#include <cstddef>
#include <vector>

struct ValidationStats {
    std::size_t total = 0;
    std::size_t offside_count = 0;
    std::size_t offside_margin_under_0_3 = 0;
    std::size_t offside_margin_over_1_0 = 0;
    std::size_t daylight_flips = 0;
    double sum_margin_offside = 0.0;
    double max_offside_margin = 0.0;
    float daylight_gap = 0.025f;
};

std::vector<Scenario> generate_scenarios(std::size_t count, RNG &rng, ValidationStats &stats);

