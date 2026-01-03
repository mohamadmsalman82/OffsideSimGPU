#include "cpu_simulation.hpp"
#include "gpu_simulation.hpp"
#include "loader.hpp"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    try {
        std::string path = "../scenarios.bin";
        std::size_t max_count = 0;
        if (argc >= 2) {
            path = argv[1];
        }
        if (argc >= 3) {
            max_count = static_cast<std::size_t>(std::strtoull(argv[2], nullptr, 10));
        }

        auto scenarios = load_scenarios(path, max_count);
        std::cout << "Loaded " << scenarios.size() << " scenarios\n";

        auto cpu_res = run_cpu_simulation(scenarios);
        std::cout << "CPU simulation complete\n";

        auto gpu_res = run_gpu_simulation(scenarios);
        std::cout << "GPU simulation complete\n";

        if (cpu_res.decisions.size() != gpu_res.decisions.size()) {
            std::cerr << "Validation FAILED: decision array sizes differ\n";
            return 1;
        }

        for (std::size_t i = 0; i < cpu_res.decisions.size(); ++i) {
            if (cpu_res.decisions[i] != gpu_res.decisions[i]) {
                std::size_t scenario_idx = i / 5;
                std::size_t rule_idx = i % 5;
                std::cerr << "Validation FAILED at scenario " << scenario_idx << " rule " << rule_idx << "\n";
                return 1;
            }
        }

        std::cout << "Validation PASSED\n\n";

        const char *rule_names[5] = {"FIFA", "Daylight", "Torso", "Feet", "Tolerance+10cm"};
        for (int i = 0; i < 5; ++i) {
            std::cout << std::left << std::setw(12) << rule_names[i] << ": " << cpu_res.counts[static_cast<std::size_t>(i)] << " offsides\n";
        }
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "\nCPU time: " << cpu_res.millis << " ms\n";
        std::cout << "GPU kernel time: " << gpu_res.kernel_ms << " ms\n";
        std::cout << "GPU total time (incl. memcpy): " << gpu_res.total_ms << " ms\n";
        if (gpu_res.total_ms > 0.0) {
            std::cout << "Speedup (CPU / GPU total): " << (cpu_res.millis / gpu_res.total_ms) << "x\n";
        }

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

