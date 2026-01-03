#pragma once

#include "scenario.hpp"

#include <vector>

struct PlayerSoAHost {
    std::vector<float> torso_x;
    std::vector<float> torso_y;
    std::vector<float> hip_x;
    std::vector<float> hip_y;
    std::vector<float> left_foot_x;
    std::vector<float> left_foot_y;
    std::vector<float> right_foot_x;
    std::vector<float> right_foot_y;
    std::vector<float> head_x;
    std::vector<float> head_y;

    void resize(std::size_t elements) {
        torso_x.resize(elements);
        torso_y.resize(elements);
        hip_x.resize(elements);
        hip_y.resize(elements);
        left_foot_x.resize(elements);
        left_foot_y.resize(elements);
        right_foot_x.resize(elements);
        right_foot_y.resize(elements);
        head_x.resize(elements);
        head_y.resize(elements);
    }
};

struct PlayerSoADevice {
    float *torso_x{};
    float *torso_y{};
    float *hip_x{};
    float *hip_y{};
    float *left_foot_x{};
    float *left_foot_y{};
    float *right_foot_x{};
    float *right_foot_y{};
    float *head_x{};
    float *head_y{};
};

inline void pack_attackers_soa(const std::vector<Scenario> &scenarios, PlayerSoAHost &out) {
    const std::size_t players = scenarios.size() * 11;
    out.resize(players);
    for (std::size_t s = 0; s < scenarios.size(); ++s) {
        for (int p = 0; p < 11; ++p) {
            const std::size_t idx = s * 11 + static_cast<std::size_t>(p);
            const Player &pl = scenarios[s].attackers[p];
            out.torso_x[idx] = pl.torso_x;
            out.torso_y[idx] = pl.torso_y;
            out.hip_x[idx] = pl.hip_x;
            out.hip_y[idx] = pl.hip_y;
            out.left_foot_x[idx] = pl.left_foot_x;
            out.left_foot_y[idx] = pl.left_foot_y;
            out.right_foot_x[idx] = pl.right_foot_x;
            out.right_foot_y[idx] = pl.right_foot_y;
            out.head_x[idx] = pl.head_x;
            out.head_y[idx] = pl.head_y;
        }
    }
}

inline void pack_defenders_soa(const std::vector<Scenario> &scenarios, PlayerSoAHost &out) {
    const std::size_t players = scenarios.size() * 11;
    out.resize(players);
    for (std::size_t s = 0; s < scenarios.size(); ++s) {
        for (int p = 0; p < 11; ++p) {
            const std::size_t idx = s * 11 + static_cast<std::size_t>(p);
            const Player &pl = scenarios[s].defenders[p];
            out.torso_x[idx] = pl.torso_x;
            out.torso_y[idx] = pl.torso_y;
            out.hip_x[idx] = pl.hip_x;
            out.hip_y[idx] = pl.hip_y;
            out.left_foot_x[idx] = pl.left_foot_x;
            out.left_foot_y[idx] = pl.left_foot_y;
            out.right_foot_x[idx] = pl.right_foot_x;
            out.right_foot_y[idx] = pl.right_foot_y;
            out.head_x[idx] = pl.head_x;
            out.head_y[idx] = pl.head_y;
        }
    }
}

