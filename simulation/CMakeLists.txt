cmake_minimum_required(VERSION 3.18)
project(offside_sim LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(SRC
    loader.cpp
    cpu_simulation.cpp
    gpu_simulation.cu
    main.cpp
)

add_executable(offside_sim ${SRC})
target_include_directories(offside_sim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(offside_sim PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

target_compile_features(offside_sim PRIVATE cxx_std_17)

