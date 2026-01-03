#pragma once

#include "scenario.hpp"

#include <array>
#include <vector>

struct CpuResult {
    std::array<int, 5> counts{{0, 0, 0, 0, 0}};
    double millis = 0.0;
    std::vector<int> decisions; // per-scenario x 5 flattened
};

CpuResult run_cpu_simulation(const std::vector<Scenario> &scenarios);

