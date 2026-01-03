#include "generator.hpp"
#include "rng.hpp"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

int main(int argc, char **argv) {
    std::size_t scenario_count = 10000;
    std::uint64_t seed = 1337ULL;

    if (argc >= 2) {
        scenario_count = static_cast<std::size_t>(std::strtoull(argv[1], nullptr, 10));
    }
    if (argc >= 3) {
        seed = std::strtoull(argv[2], nullptr, 10);
    }

    RNG rng(seed);
    ValidationStats stats{};
    auto scenarios = generate_scenarios(scenario_count, rng, stats);

    std::ofstream bin("scenarios.bin", std::ios::binary);
    bin.write(reinterpret_cast<const char *>(scenarios.data()),
              static_cast<std::streamsize>(scenarios.size() * sizeof(Scenario)));
    if (!bin) {
        std::cerr << "Failed to write scenarios.bin\n";
        return 1;
    }
    bin.close();

    const double offside_rate = stats.total ? static_cast<double>(stats.offside_count) / static_cast<double>(stats.total) : 0.0;
    const double mean_offside_margin = stats.offside_count ? stats.sum_margin_offside / static_cast<double>(stats.offside_count) : 0.0;
    const double small_margin_rate = stats.offside_count ? static_cast<double>(stats.offside_margin_under_0_3) / static_cast<double>(stats.offside_count) : 0.0;
    const double large_margin_rate = stats.offside_count ? static_cast<double>(stats.offside_margin_over_1_0) / static_cast<double>(stats.offside_count) : 0.0;
    const double daylight_flip_rate = stats.offside_count ? static_cast<double>(stats.daylight_flips) / static_cast<double>(stats.offside_count) : 0.0;

    std::ofstream meta("metadata.txt");
    meta << "seed=" << seed << "\n";
    meta << "scenario_count=" << scenarios.size() << "\n";
    meta << "offside_rate=" << offside_rate << "\n";
    meta << "mean_offside_margin_m=" << mean_offside_margin << "\n";
    meta << "offside_lt_0_3m_ratio=" << small_margin_rate << "\n";
    meta << "offside_gt_1_0m_ratio=" << large_margin_rate << "\n";
    meta << "daylight_gap_m=" << stats.daylight_gap << "\n";
    meta << "daylight_flip_ratio=" << daylight_flip_rate << "\n";
    meta.close();

    std::cout << "Generated " << scenarios.size() << " scenarios\n";
    std::cout << "RNG seed: " << seed << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Offside rate: " << offside_rate * 100.0 << "%\n";
    std::cout << "Mean offside margin: " << mean_offside_margin * 100.0 << " cm\n";
    std::cout << "Offsides <30cm: " << small_margin_rate * 100.0 << "% of offsides\n";
    std::cout << "Offsides >1m: " << large_margin_rate * 100.0 << "% of offsides\n";
    std::cout << "Daylight rule flips (@ " << stats.daylight_gap * 100.0 << " cm gap): "
              << daylight_flip_rate * 100.0 << "% of offsides\n";

    return 0;
}

