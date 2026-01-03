#include "gpu_simulation.hpp"

#include "soa.hpp"

#include <cuda_runtime.h>

#include <array>
#include <stdexcept>
#include <vector>

#define CUDA_CHECK(call)                                                                 \
    do {                                                                                 \
        cudaError_t err = (call);                                                        \
        if (err != cudaSuccess) {                                                        \
            throw std::runtime_error(std::string("CUDA error: ") + cudaGetErrorString(err)); \
        }                                                                                \
    } while (0)

namespace {

__device__ inline float max4(float a, float b, float c, float d) {
    float m1 = a > b ? a : b;
    float m2 = c > d ? c : d;
    return m1 > m2 ? m1 : m2;
}

__device__ inline float max_playable(float head_x, float hip_x, float lf_x, float rf_x) {
    return max4(head_x, hip_x, lf_x, rf_x);
}

struct PlayerSoADeviceView {
    const float *torso_x;
    const float *torso_y;
    const float *hip_x;
    const float *hip_y;
    const float *left_foot_x;
    const float *left_foot_y;
    const float *right_foot_x;
    const float *right_foot_y;
    const float *head_x;
    const float *head_y;
};

__global__ void offsideKernel(PlayerSoADeviceView attackers,
                              PlayerSoADeviceView defenders,
                              const int *scorer_ids,
                              int *results,
                              int num_scenarios) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_scenarios) return;

    // Find second-last defender (second smallest torso_x)
    float first = 1e9f;
    float second = 1e9f;
    int first_idx = -1;
    int second_idx = 0;
    const int base_def = idx * 11;
    for (int i = 0; i < 11; ++i) {
        float tx = defenders.torso_x[base_def + i];
        if (tx < first) {
            second = first;
            second_idx = first_idx;
            first = tx;
            first_idx = i;
        } else if (tx < second) {
            second = tx;
            second_idx = i;
        }
    }

    const int base_att = idx * 11;
    const int scorer = scorer_ids[idx];

    // Attacker body parts
    const float att_head = attackers.head_x[base_att + scorer];
    const float att_hip = attackers.hip_x[base_att + scorer];
    const float att_lf = attackers.left_foot_x[base_att + scorer];
    const float att_rf = attackers.right_foot_x[base_att + scorer];

    // Defender (second-last) body parts
    const float def_head = defenders.head_x[base_def + second_idx];
    const float def_hip = defenders.hip_x[base_def + second_idx];
    const float def_lf = defenders.left_foot_x[base_def + second_idx];
    const float def_rf = defenders.right_foot_x[base_def + second_idx];

    const float attacker_max = max_playable(att_head, att_hip, att_lf, att_rf);
    const float defender_max = max_playable(def_head, def_hip, def_lf, def_rf);

    const float att_feet = att_lf > att_rf ? att_lf : att_rf;
    const float def_feet = def_lf > def_rf ? def_lf : def_rf;

    int out_base = idx * 5;
    // Rule 1: FIFA
    results[out_base + 0] = attacker_max > defender_max;
    // Rule 2: Daylight
    results[out_base + 1] = attacker_max > (defender_max + 0.01f);
    // Rule 3: Torso
    results[out_base + 2] = att_hip > def_hip;
    // Rule 4: Feet
    results[out_base + 3] = att_feet > def_feet;
    // Rule 5: +10cm tolerance
    results[out_base + 4] = (attacker_max - defender_max) > 0.10f;
}

template <typename T>
T *device_alloc_and_copy(const std::vector<T> &host) {
    T *dev = nullptr;
    CUDA_CHECK(cudaMalloc(&dev, host.size() * sizeof(T)));
    CUDA_CHECK(cudaMemcpy(dev, host.data(), host.size() * sizeof(T), cudaMemcpyHostToDevice));
    return dev;
}

} // namespace

GpuResult run_gpu_simulation(const std::vector<Scenario> &scenarios) {
    GpuResult result{};
    if (scenarios.empty()) {
        return result;
    }

    const int num_scenarios = static_cast<int>(scenarios.size());
    PlayerSoAHost att_host;
    PlayerSoAHost def_host;
    pack_attackers_soa(scenarios, att_host);
    pack_defenders_soa(scenarios, def_host);

    std::vector<int> scorer_ids(scenarios.size());
    for (std::size_t i = 0; i < scenarios.size(); ++i) {
        scorer_ids[i] = scenarios[i].scorer_id;
    }

    int *d_results = nullptr;
    PlayerSoADeviceView att_dev{};
    PlayerSoADeviceView def_dev{};
    int *d_scorer_ids = nullptr;

    cudaEvent_t start_total, stop_total, start_kernel, stop_kernel;
    CUDA_CHECK(cudaEventCreate(&start_total));
    CUDA_CHECK(cudaEventCreate(&stop_total));
    CUDA_CHECK(cudaEventCreate(&start_kernel));
    CUDA_CHECK(cudaEventCreate(&stop_kernel));

    CUDA_CHECK(cudaEventRecord(start_total));

    // Allocate and copy SoA
    att_dev.torso_x = device_alloc_and_copy(att_host.torso_x);
    att_dev.torso_y = device_alloc_and_copy(att_host.torso_y);
    att_dev.hip_x = device_alloc_and_copy(att_host.hip_x);
    att_dev.hip_y = device_alloc_and_copy(att_host.hip_y);
    att_dev.left_foot_x = device_alloc_and_copy(att_host.left_foot_x);
    att_dev.left_foot_y = device_alloc_and_copy(att_host.left_foot_y);
    att_dev.right_foot_x = device_alloc_and_copy(att_host.right_foot_x);
    att_dev.right_foot_y = device_alloc_and_copy(att_host.right_foot_y);
    att_dev.head_x = device_alloc_and_copy(att_host.head_x);
    att_dev.head_y = device_alloc_and_copy(att_host.head_y);

    def_dev.torso_x = device_alloc_and_copy(def_host.torso_x);
    def_dev.torso_y = device_alloc_and_copy(def_host.torso_y);
    def_dev.hip_x = device_alloc_and_copy(def_host.hip_x);
    def_dev.hip_y = device_alloc_and_copy(def_host.hip_y);
    def_dev.left_foot_x = device_alloc_and_copy(def_host.left_foot_x);
    def_dev.left_foot_y = device_alloc_and_copy(def_host.left_foot_y);
    def_dev.right_foot_x = device_alloc_and_copy(def_host.right_foot_x);
    def_dev.right_foot_y = device_alloc_and_copy(def_host.right_foot_y);
    def_dev.head_x = device_alloc_and_copy(def_host.head_x);
    def_dev.head_y = device_alloc_and_copy(def_host.head_y);

    d_scorer_ids = device_alloc_and_copy(scorer_ids);

    CUDA_CHECK(cudaMalloc(&d_results, static_cast<std::size_t>(num_scenarios) * 5 * sizeof(int)));

    const int threads = 256;
    const int blocks = (num_scenarios + threads - 1) / threads;

    CUDA_CHECK(cudaEventRecord(start_kernel));
    offsideKernel<<<blocks, threads>>>(att_dev, def_dev, d_scorer_ids, d_results, num_scenarios);
    CUDA_CHECK(cudaEventRecord(stop_kernel));
    CUDA_CHECK(cudaGetLastError());

    std::vector<int> host_results(static_cast<std::size_t>(num_scenarios) * 5);
    CUDA_CHECK(cudaMemcpy(host_results.data(), d_results,
                          static_cast<std::size_t>(num_scenarios) * 5 * sizeof(int),
                          cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaEventRecord(stop_total));
    CUDA_CHECK(cudaEventSynchronize(stop_total));
    CUDA_CHECK(cudaEventSynchronize(stop_kernel));

    float kernel_ms = 0.0f;
    float total_ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&kernel_ms, start_kernel, stop_kernel));
    CUDA_CHECK(cudaEventElapsedTime(&total_ms, start_total, stop_total));

    result.kernel_ms = kernel_ms;
    result.total_ms = total_ms;
    result.decisions = std::move(host_results);

    for (int i = 0; i < num_scenarios; ++i) {
        for (int r = 0; r < 5; ++r) {
            result.counts[static_cast<std::size_t>(r)] += result.decisions[static_cast<std::size_t>(i) * 5 + static_cast<std::size_t>(r)];
        }
    }

    // Cleanup
    cudaFree(att_dev.torso_x);
    cudaFree(att_dev.torso_y);
    cudaFree(att_dev.hip_x);
    cudaFree(att_dev.hip_y);
    cudaFree(att_dev.left_foot_x);
    cudaFree(att_dev.left_foot_y);
    cudaFree(att_dev.right_foot_x);
    cudaFree(att_dev.right_foot_y);
    cudaFree(att_dev.head_x);
    cudaFree(att_dev.head_y);

    cudaFree(def_dev.torso_x);
    cudaFree(def_dev.torso_y);
    cudaFree(def_dev.hip_x);
    cudaFree(def_dev.hip_y);
    cudaFree(def_dev.left_foot_x);
    cudaFree(def_dev.left_foot_y);
    cudaFree(def_dev.right_foot_x);
    cudaFree(def_dev.right_foot_y);
    cudaFree(def_dev.head_x);
    cudaFree(def_dev.head_y);

    cudaFree(d_scorer_ids);
    cudaFree(d_results);

    cudaEventDestroy(start_total);
    cudaEventDestroy(stop_total);
    cudaEventDestroy(start_kernel);
    cudaEventDestroy(stop_kernel);

    return result;
}

