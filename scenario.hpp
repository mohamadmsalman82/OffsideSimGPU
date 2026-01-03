#pragma once

#include "player.hpp"

#include <cstdint>
#include <type_traits>

struct alignas(16) Scenario {
    Player attackers[11];
    Player defenders[11];
    std::int32_t scorer_id;
};

static_assert(std::is_trivially_copyable<Scenario>::value, "Scenario must be trivially copyable");
static_assert(std::is_standard_layout<Scenario>::value, "Scenario must be standard layout");

