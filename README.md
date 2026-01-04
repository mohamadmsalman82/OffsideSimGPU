# GPU-Accelerated Offside Rule Analysis  
### Evaluating Proposed FIFA Offside Rule Changes at Scale Using CUDA

---

## Overview

Offside decisions are among the most controversial and impactful rulings in modern football (soccer). With the introduction of VAR, decisions are now often made based on margins of just a few centimeters—raising concerns about fairness, flow of the game, and spectator experience.

Ahead of the **2026 FIFA World Cup**, FIFA and IFAB have publicly discussed multiple **proposed changes to the offside rule**, including the so-called *“daylight rule”* and restrictions on which body parts should count for offside.

This project answers a fundamental question:

> **How would different offside rule definitions change real match outcomes if applied consistently at scale?**

To answer this, I built a **GPU-accelerated simulation system** that evaluates hundreds of thousands of offside scenarios under multiple rule variants, comparing both **sporting impact** and **computational performance** between CPU and GPU implementations.

---

## Soccer / Football Context

### The Current Offside Rule (Simplified)

Under the current FIFA Laws of the Game:
- A player is offside if **any playable part of the body** (excluding arms) is **ahead of the second-last defender** at the moment the ball is played.

In practice:
- Head, torso, hips, knees, and feet can all trigger offside
- Decisions often hinge on **centimeter-level margins**
- VAR freeze-frames are used to determine legality

---

### Proposed Rule Variants Studied

This project evaluates **five offside definitions**, all discussed in professional football discourse:

1. **Current FIFA Rule**  
   Any playable body part beyond the second-last defender → offside.

2. **Daylight Rule**  
   The attacker must be *clearly* ahead — a visible gap (“daylight”) must exist.

3. **Torso-Only Rule**  
   Only the torso / hips are considered for offside decisions.

4. **Feet-Only Rule**  
   Only the feet are considered (common intuition in attacking play).

5. **Tolerance Rule (+10 cm)**  
   A margin of error is applied to account for measurement uncertainty.

Each rule reflects a different philosophy:
- Should attackers be given more benefit of the doubt?
- Should marginal body parts like shoulders decide goals?
- How sensitive are outcomes to measurement noise?

---

## Synthetic Match Scenario Generation

### Why Synthetic Data?

Professional optical-tracking data (e.g., TRACAB, Second Spectrum) is:
- Proprietary
- Extremely expensive
- Not publicly redistributable

However, **the computational problem is identical** regardless of data source.

To address this, I built a **procedural synthetic data generator** that produces realistic offside scenarios based on known football dynamics.

---

### What Each Scenario Contains

Each generated scenario represents the **exact moment a pass is played**, including:

- **22 players** (11 attackers, 11 defenders)
- 2D pitch coordinates (X, Y) for each player
- Separate positions for:
  - Head
  - Torso / hips
  - Left foot
  - Right foot
- Identification of the goal-scoring attacker

The generator enforces:
- Realistic formations and spacing
- Plausible defensive lines
- Offside rates aligned with professional league statistics
- Tight margin distributions (centimeter-scale)

This ensures the results reflect **real-world behavior**, not random noise.

---

## Technical Problem Statement

From a systems perspective, this is an **embarrassingly parallel geometric classification problem**:

- Each offside decision is **independent**
- Each scenario requires:
  - Finding the second-last defender
  - Comparing attacker and defender body-part extrema
  - Applying multiple rule variants

For large datasets, this becomes computationally expensive on the CPU.

---

## CPU Implementation

The baseline implementation:
- Written in modern C++ (C++17)
- Processes scenarios sequentially
- Applies all five rule checks per scenario
- Serves as a correctness and performance baseline

This version is intentionally straightforward to make performance comparisons meaningful.

---

## GPU Implementation (CUDA)

### Parallelization Strategy

- **One CUDA thread per scenario**
- Each thread evaluates all five rule variants
- Data stored in **Structure-of-Arrays (SoA)** format for memory coalescing
- All geometric comparisons are branch-light and arithmetic-heavy

This maps naturally to the GPU execution model.

---

### CUDA Optimizations Applied

The GPU implementation is not merely “ported” — it is **optimized**:

- Structure-of-Arrays layout for coalesced global memory access
- Avoidance of unnecessary sorting (O(n) min-finding instead of O(n log n))
- Separation of kernel time vs total time (including PCIe transfers)
- Validation against CPU results for correctness

The design reflects real GPU performance engineering rather than toy CUDA usage.

---

## Performance Evaluation

Benchmarks were run across increasing scenario counts to observe **scaling behavior** and the **CPU–GPU crossover point**.

### Key Observation

- For small workloads, CPU outperforms GPU due to:
  - Kernel launch overhead
  - Host-to-device memory transfer cost
- As the workload grows, GPU parallelism amortizes this overhead

This behavior is expected and explicitly documented.

### Representative Results

| Scenarios | CPU Time (ms) | GPU Kernel (ms) | GPU Total (ms) |
|----------|---------------|------------------|---------------|
| 10,000   | Fast          | Very Fast        | Slower overall |
| 100,000  | Moderate      | Fast             | Near parity   |
| 500,000  | Slow          | Very Fast        | GPU wins      |

---

## Soccer-Level Results

Across large scenario sets, the analysis shows:

- The **daylight rule** substantially reduces marginal offsides
- Torso-only interpretations favor attacking play
- Feet-only interpretations penalize high defensive lines
- Small tolerance margins significantly change offside counts

These findings highlight how **rule wording alone** can reshape match outcomes without altering player behavior.

---

## Validation

- CPU and GPU outputs are bit-for-bit consistent
- Automatic validation fails fast if discrepancies are detected
- Ensures performance gains do not come at the cost of correctness

---

## Why This Project Matters

### From a Football Perspective
- Quantifies the real impact of rule changes
- Moves debates from anecdotes to data
- Demonstrates how technology shapes modern officiating

### From a Systems Perspective
- Demonstrates GPU-appropriate problem selection
- Shows real performance tradeoffs
- Applies CUDA in a domain-relevant, non-toy context

---

## Build & Run (Summary)

- **Language:** C++17 + CUDA  
- **Build system:** CMake + Ninja (Windows)  
- **GPU:** NVIDIA CUDA-capable GPU required  
- CPU-only components can run without CUDA

(See detailed build instructions in the repository.)

---

## Final Note

This project intentionally sits at the intersection of:
- **sports analytics**
- **high-performance computing**
- **systems-level engineering**

The goal is not just to simulate football rules, but to demonstrate how **parallel computing enables analysis at scales that are otherwise impractical**.
