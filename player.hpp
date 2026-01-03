#pragma once

#include "rng.hpp"

#include <algorithm>

constexpr float PITCH_LENGTH = 105.0f;
constexpr float PITCH_WIDTH = 68.0f;

inline float clamp_pitch_x(float v) {
    return std::max(0.0f, std::min(PITCH_LENGTH, v));
}

inline float clamp_pitch_y(float v) {
    return std::max(0.0f, std::min(PITCH_WIDTH, v));
}

struct alignas(16) Player {
    float torso_x, torso_y;
    float hip_x, hip_y;
    float left_foot_x, left_foot_y;
    float right_foot_x, right_foot_y;
    float head_x, head_y;
};

inline Player make_player_from_torso(float torso_x, float torso_y, RNG &rng) {
    Player p{};
    p.torso_x = clamp_pitch_x(torso_x);
    p.torso_y = clamp_pitch_y(torso_y);

    const float foot_dx = rng.uniform(-0.35f, -0.10f);
    const float left_dy = rng.uniform(-0.12f, -0.05f);
    const float right_dy = rng.uniform(0.05f, 0.12f);

    const float hip_dx = rng.uniform(0.05f, 0.12f);
    const float head_dx = rng.uniform(0.20f, 0.35f);
    const float head_dy = rng.uniform(-0.06f, 0.06f);

    p.hip_x = clamp_pitch_x(p.torso_x + hip_dx);
    p.hip_y = clamp_pitch_y(p.torso_y);

    p.left_foot_x = clamp_pitch_x(p.torso_x + foot_dx);
    p.left_foot_y = clamp_pitch_y(p.torso_y + left_dy);

    p.right_foot_x = clamp_pitch_x(p.torso_x + foot_dx);
    p.right_foot_y = clamp_pitch_y(p.torso_y + right_dy);

    p.head_x = clamp_pitch_x(p.torso_x + head_dx);
    p.head_y = clamp_pitch_y(p.torso_y + head_dy);
    return p;
}

inline float playable_max_x(const Player &p) {
    float limb_max = std::max(p.left_foot_x, p.right_foot_x);
    limb_max = std::max(limb_max, p.hip_x);
    limb_max = std::max(limb_max, p.head_x);
    return limb_max;
}

