OffsideSimGPU
==============

Synthetic football offside generator plus a CPU/GPU validator. The top-level program creates realistic 11v11 frames and writes them to `scenarios.bin` (with summary stats in `metadata.txt`). The `simulation/` target loads that file, runs five offside-rule variants on both CPU and CUDA, and checks the decisions match while reporting timing.


What's here
-----------
- `main.cpp`, `generator.cpp/hpp`, `player.hpp`, `scenario.hpp`, `rng.hpp`: scenario generator and data layout.
- `scenarios.bin`, `metadata.txt`: sample output produced by the generator.
- `simulation/`: CMake project for CPU and CUDA simulation/validation.


Requirements
------------
- C++17 compiler (tested with `g++`/`clang++`).
- CUDA toolkit (nvcc) and a compatible NVIDIA GPU for the GPU path.
- CMake ≥ 3.18 for the simulator build.


Quick start
-----------
1) Build the generator (top-level):
```
g++ -std=c++17 -O2 main.cpp generator.cpp -o offside_gen
```

2) Generate scenarios (args: `<count> [seed]`, defaults 10000 and 1337):
```
./offside_gen 20000 42
```
Outputs:
- `scenarios.bin` containing packed `Scenario` structs.
- `metadata.txt` summarizing offside rates/margins and the RNG seed.

3) Build the simulator (CPU + CUDA):
```
cd simulation
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

4) Run validation (args: `<path_to_bin> [max_scenarios]`, defaults to `../scenarios.bin` and all records):
```
./build/offside_sim ../scenarios.bin
```
What you’ll see:
- CPU and GPU both compute offside flags for five rule variants (FIFA, Daylight, Torso-only, Feet-only, +10cm tolerance).
- Run fails fast if any CPU/GPU decision disagrees; otherwise prints per-rule offside counts plus CPU time, GPU kernel time, and total GPU time (incl. memcopies) with a speedup ratio.


Data shape (for downstream use)
--------------------------------
- `Scenario`: 11 attackers + 11 defenders + `scorer_id` (the attacking player taking the shot). Trivially copyable, 16-byte aligned.
- `Player`: torso, hip, both feet, and head coordinates within a 105m x 68m pitch; helper `playable_max_x` finds the furthest playable body part.


Repro tips
----------
- The RNG seed is printed and stored in `metadata.txt`; rerun with the same seed to reproduce a dataset.
- Change scenario count/seed via CLI args; `max_scenarios` in the simulator lets you cap the loaded set for quick runs.


Troubleshooting
---------------
- CMake or nvcc not found: ensure the CUDA toolkit and a GPU driver are installed and visible in `PATH`/`CUDA_HOME`.
- CUDA runtime errors at launch: check that your GPU supports the compiled compute capability and that you’re not running out of device memory.
- Mismatched CPU/GPU decisions: the program will exit with the first mismatch, printing the scenario index and rule index to help debug.

