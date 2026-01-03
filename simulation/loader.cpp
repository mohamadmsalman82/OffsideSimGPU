#include "loader.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

std::vector<Scenario> load_scenarios(const std::string &path, std::size_t max_count) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        throw std::runtime_error("Failed to open scenarios file: " + path);
    }
    f.seekg(0, std::ios::end);
    const std::streamoff end = f.tellg();
    f.seekg(0, std::ios::beg);
    if (end <= 0) {
        throw std::runtime_error("Empty scenarios file: " + path);
    }
    const std::size_t total_bytes = static_cast<std::size_t>(end);
    if (total_bytes % sizeof(Scenario) != 0) {
        throw std::runtime_error("Invalid scenarios.bin size (not divisible by Scenario size)");
    }
    std::size_t total_scenarios = total_bytes / sizeof(Scenario);
    if (max_count > 0 && max_count < total_scenarios) {
        total_scenarios = max_count;
    }
    std::vector<Scenario> scenarios(total_scenarios);
    f.read(reinterpret_cast<char *>(scenarios.data()), static_cast<std::streamsize>(total_scenarios * sizeof(Scenario)));
    if (!f) {
        throw std::runtime_error("Failed to read scenario data");
    }
    return scenarios;
}

