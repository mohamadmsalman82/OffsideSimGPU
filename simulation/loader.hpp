#pragma once

#include "scenario.hpp"

#include <string>
#include <vector>

std::vector<Scenario> load_scenarios(const std::string &path, std::size_t max_count = 0);

