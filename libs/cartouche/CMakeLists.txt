# Copyright 2025 Samuel "Lord-Windy" Brown
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Use FetchContent for simpler dependency management
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.18
    CMAKE_ARGS -DSDL_TEST=OFF
)

FetchContent_Declare(
    clay
    GIT_REPOSITORY https://github.com/nicbarker/clay.git
    GIT_TAG v0.14
)

FetchContent_Declare(
    wgpu
    GIT_REPOSITORY https://github.com/gfx-rs/wgpu-native.git
    GIT_TAG 25.0.2.1
)

FetchContent_MakeAvailable(SDL3 clay)

# wgpu-native needs special handling as it may not have CMake support
FetchContent_GetProperties(wgpu)
if(NOT wgpu_POPULATED)
    FetchContent_Populate(wgpu)
    # Add webgpu as interface library with headers only for now
    add_library(wgpu INTERFACE)
    target_include_directories(wgpu INTERFACE ${wgpu_SOURCE_DIR}/include)
endif()

# Clay needs to be set up as interface library
add_library(clay INTERFACE)
target_include_directories(clay INTERFACE ${clay_SOURCE_DIR})

# Cartouche library
ptah_add_library(cartouche
    SOURCES
        src/cartouche.c
    PUBLIC_HEADERS
        include/cartouche.h
    DEPENDENCIES
        SDL3::SDL3
        clay
        wgpu
)

# Demo application
ptah_add_executable(cartouche_demo
    SOURCES
        src/main.c
    DEPENDENCIES
        cartouche
)