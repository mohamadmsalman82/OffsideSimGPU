#include "generator.hpp"

#include "player.hpp"
#include "scenario.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace {

float clamp_value(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

enum class Lane { Left, Center, Right };

float sample_defender_lane_y(Lane lane, RNG &rng) {
    float y = 34.0f;
    switch (lane) {
    case Lane::Left:
        y = rng.uniform(15.0f, 25.0f);
        break;
    case Lane::Center:
        y = rng.uniform(30.0f, 38.0f);
        break;
    case Lane::Right:
        y = rng.uniform(43.0f, 53.0f);
        break;
    }
    y += rng.normal(0.0f, 0.8f);
    return clamp_pitch_y(y);
}

float sample_attacker_front_y(int idx, int front_count, RNG &rng) {
    if (front_count == 3) {
        if (idx == 0) {
            return clamp_pitch_y(rng.uniform(30.0f, 40.0f)); // central striker
        }
        if (idx == 1) {
            return clamp_pitch_y(rng.uniform(10.0f, 24.0f)); // left winger
        }
        return clamp_pitch_y(rng.uniform(44.0f, 60.0f)); // right winger
    }
    // four-player front: wide, two central, wide
    switch (idx) {
    case 0:
        return clamp_pitch_y(rng.uniform(12.0f, 24.0f)); // left
    case 1:
        return clamp_pitch_y(rng.uniform(30.0f, 38.0f)); // central 1
    case 2:
        return clamp_pitch_y(rng.uniform(34.0f, 44.0f)); // central 2
    default:
        return clamp_pitch_y(rng.uniform(44.0f, 60.0f)); // right
    }
}

float sample_attacker_support_y(int idx, RNG &rng) {
    static const std::array<std::pair<float, float>, 7> ranges = {{
        {6.0f, 22.0f},
        {46.0f, 62.0f},
        {24.0f, 40.0f},
        {12.0f, 28.0f},
        {40.0f, 56.0f},
        {28.0f, 44.0f},
        {14.0f, 30.0f},
    }};
    auto range = ranges[static_cast<std::size_t>(idx % ranges.size())];
    return clamp_pitch_y(rng.uniform(range.first, range.second));
}

void generate_defenders(RNG &rng, float defensive_line_x, Player (&defenders)[11]) {
    static const std::array<Lane, 10> lane_pattern = {
        Lane::Left, Lane::Center, Lane::Right, Lane::Left, Lane::Center,
        Lane::Right, Lane::Center, Lane::Center, Lane::Left, Lane::Right};

    for (std::size_t i = 0; i < 10; ++i) {
        const float x = clamp_pitch_x(defensive_line_x + rng.normal(0.0f, 0.5f));
        const float y = sample_defender_lane_y(lane_pattern[i], rng);
        defenders[i] = make_player_from_torso(x, y, rng);
    }

    // Goalkeeper positioning close to the goal line with slight lateral bias.
    const float gk_x = clamp_pitch_x(5.0f + rng.normal(0.0f, 1.0f));
    const float gk_y = clamp_pitch_y(34.0f + rng.normal(0.0f, 1.5f));
    defenders[10] = make_player_from_torso(gk_x, gk_y, rng);
}

float compute_offside_line(const Player (&defenders)[11]) {
    std::array<int, 11> idx{};
    for (int i = 0; i < 11; ++i) {
        idx[static_cast<std::size_t>(i)] = i;
    }
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return defenders[static_cast<std::size_t>(a)].torso_x < defenders[static_cast<std::size_t>(b)].torso_x;
    });
    const int second_last = idx[1]; // second closest to defending goal at x=0
    return defenders[static_cast<std::size_t>(second_last)].torso_x;
}

int select_scorer(const Player (&attackers)[11], RNG &rng) {
    std::array<int, 11> idx{};
    for (int i = 0; i < 11; ++i) {
        idx[static_cast<std::size_t>(i)] = i;
    }
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return attackers[static_cast<std::size_t>(a)].torso_x > attackers[static_cast<std::size_t>(b)].torso_x;
    });

    if (rng.bernoulli(0.7f)) {
        return idx[0];
    }
    const int choice_pool = std::min(3, 11);
    const int alt = rng.int_range(1, choice_pool - 1);
    return idx[alt];
}

void generate_attackers(RNG &rng, float offside_line_x, Player (&attackers)[11]) {
    const int front_count = rng.bernoulli(0.5f) ? 4 : 3;

    // Lead attacker straddling the offside line with calibrated aggression.
    const bool aggressive_timing = rng.bernoulli(0.6f);
    const float lead_mean = aggressive_timing ? -0.08f : -0.25f;
    const float lead_std = aggressive_timing ? 0.12f : 0.07f;
    float lead_delta = clamp_value(rng.normal(lead_mean, lead_std), -0.5f, 0.5f);
    float lead_x = clamp_pitch_x(offside_line_x + lead_delta);
    float lead_y = sample_attacker_front_y(0, front_count, rng);
    attackers[0] = make_player_from_torso(lead_x, lead_y, rng);

    // Remaining front-line attackers sit slightly deeper.
    for (int i = 1; i < front_count; ++i) {
        const float mean = -0.25f;
        const float stddev = 0.08f;
        float delta = clamp_value(rng.normal(mean, stddev), -0.55f, 0.2f);
        float torso_x = clamp_pitch_x(offside_line_x + delta);
        float torso_y = sample_attacker_front_y(i, front_count, rng);
        attackers[static_cast<std::size_t>(i)] = make_player_from_torso(torso_x, torso_y, rng);
    }

    // Support runners 2–8m behind the offside line with mild jitter.
    for (int i = front_count; i < 11; ++i) {
        float back_offset = rng.uniform(2.0f, 8.0f);
        float jitter = rng.normal(0.0f, 0.3f);
        float torso_x = clamp_pitch_x(offside_line_x - back_offset + jitter);
        float torso_y = sample_attacker_support_y(i - front_count, rng);
        attackers[static_cast<std::size_t>(i)] = make_player_from_torso(torso_x, torso_y, rng);
    }
}

void update_validation(const Scenario &scenario,
                       const std::array<int, 11> &sorted_def_idx,
                       int scorer_idx,
                       ValidationStats &stats) {
    const Player &attacker = scenario.attackers[static_cast<std::size_t>(scorer_idx)];
    const Player &second_last_def = scenario.defenders[static_cast<std::size_t>(sorted_def_idx[1])];

    const float attacker_max = playable_max_x(attacker);
    const float defender_max = playable_max_x(second_last_def);
    const float margin = attacker_max - defender_max;
    const bool offside = margin > 0.0f;
    const bool daylight_offside = margin > stats.daylight_gap;

    stats.total++;
    if (offside) {
        stats.offside_count++;
        stats.sum_margin_offside += static_cast<double>(margin);
        stats.max_offside_margin = std::max(stats.max_offside_margin, static_cast<double>(margin));
        if (margin < 0.3f) {
            stats.offside_margin_under_0_3++;
        }
        if (margin > 1.0f) {
            stats.offside_margin_over_1_0++;
        }
        if (!daylight_offside) {
            stats.daylight_flips++;
        }
    }
}

} // namespace

std::vector<Scenario> generate_scenarios(std::size_t count, RNG &rng, ValidationStats &stats) {
    std::vector<Scenario> scenarios;
    scenarios.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        Scenario scenario{};

        // 1. Tactical context: defensive line height.
        float defensive_line_x = clamp_value(rng.normal(30.0f, 4.0f), 20.0f, 45.0f);

        // 2. Defender placement.
        generate_defenders(rng, defensive_line_x, scenario.defenders);

        // 3. Offside line from second-last defender.
        std::array<int, 11> def_idx{};
        for (int j = 0; j < 11; ++j) {
            def_idx[static_cast<std::size_t>(j)] = j;
        }
        std::sort(def_idx.begin(), def_idx.end(), [&](int a, int b) {
            return scenario.defenders[static_cast<std::size_t>(a)].torso_x < scenario.defenders[static_cast<std::size_t>(b)].torso_x;
        });
        float offside_line_x = scenario.defenders[static_cast<std::size_t>(def_idx[1])].torso_x;

        // 4. Attacker placement.
        generate_attackers(rng, offside_line_x, scenario.attackers);

        // 5. Scorer selection.
        scenario.scorer_id = select_scorer(scenario.attackers, rng);

        // 6. Body parts already derived during player creation.

        // Validation / labeling (not stored in Scenario).
        update_validation(scenario, def_idx, scenario.scorer_id, stats);

        scenarios.push_back(scenario);
    }

    return scenarios;
}

