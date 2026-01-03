#pragma once

#include "scenario.hpp"

#include <array>
#include <vector>

struct GpuResult {
    std::array<int, 5> counts{{0, 0, 0, 0, 0}};
    double kernel_ms = 0.0;
    double total_ms = 0.0; // includes H2D + kernel + D2H
    std::vector<int> decisions; // per-scenario x 5 flattened
};

GpuResult run_gpu_simulation(const std::vector<Scenario> &scenarios);

