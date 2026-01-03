#include "cpu_simulation.hpp"

#include <algorithm>
#include <array>
#include <chrono>

namespace {

inline float max_playable(const Player &p) {
    return std::max(std::max(p.head_x, p.hip_x), std::max(p.left_foot_x, p.right_foot_x));
}

int find_second_last_defender(const Scenario &sc) {
    std::array<int, 11> idx{};
    for (int i = 0; i < 11; ++i) {
        idx[static_cast<std::size_t>(i)] = i;
    }
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return sc.defenders[static_cast<std::size_t>(a)].torso_x < sc.defenders[static_cast<std::size_t>(b)].torso_x;
    });
    return idx[1];
}

} // namespace

CpuResult run_cpu_simulation(const std::vector<Scenario> &scenarios) {
    CpuResult result{};
    result.decisions.resize(scenarios.size() * 5, 0);
    auto start = std::chrono::steady_clock::now();

    for (std::size_t s = 0; s < scenarios.size(); ++s) {
        const auto &sc = scenarios[s];
        const int scorer = sc.scorer_id;
        const int second_last = find_second_last_defender(sc);
        const Player &att = sc.attackers[static_cast<std::size_t>(scorer)];
        const Player &def = sc.defenders[static_cast<std::size_t>(second_last)];

        const float attacker_max = max_playable(att);
        const float defender_max = max_playable(def);

        const float att_feet = std::max(att.left_foot_x, att.right_foot_x);
        const float def_feet = std::max(def.left_foot_x, def.right_foot_x);

        // Rule 1: FIFA
        const int fifa = attacker_max > defender_max;
        const int daylight = attacker_max > defender_max + 0.01f;
        const int torso = att.hip_x > def.hip_x;
        const int feet = att_feet > def_feet;
        const int tol = (attacker_max - defender_max) > 0.10f;

        result.counts[0] += fifa;
        // Rule 2: Daylight (1 cm gap)
        result.counts[1] += daylight;
        // Rule 3: Torso only
        result.counts[2] += torso;
        // Rule 4: Feet only
        result.counts[3] += feet;
        // Rule 5: Tolerance +10 cm
        result.counts[4] += tol;

        std::size_t base = s * 5;
        result.decisions[base + 0] = fifa;
        result.decisions[base + 1] = daylight;
        result.decisions[base + 2] = torso;
        result.decisions[base + 3] = feet;
        result.decisions[base + 4] = tol;
    }

    auto end = std::chrono::steady_clock::now();
    result.millis = std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

