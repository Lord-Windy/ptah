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

# Example usage of Ptah Library Manager
# This file demonstrates how to use the Ptah system in your CMakeLists.txt

# Include the Ptah Library Manager
include(cmake/PtahLibraryManager.cmake)

# Example 1: SDL (CMake-based library)
ptah_add_library(
    NAME SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    VERSION 3.0.0  # Will resolve to tag v3.0.0
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DSDL_SHARED=OFF
        -DSDL_STATIC=ON
        -DSDL_TEST=OFF
)

# Example 2: wgpu-native (Cargo-based library)
ptah_add_library(
    NAME wgpu-native
    GIT_REPOSITORY https://github.com/gfx-rs/wgpu-native.git
    VERSION trunk  # Will track trunk branch
    BUILD_SYSTEM cargo
)

# Example 3: Clay (Header-only library)
ptah_add_library(
    NAME clay
    GIT_REPOSITORY https://github.com/nicbarker/clay.git
    VERSION main
    HEADER_ONLY TRUE
    INTERFACE_TARGET TRUE
)

# Example 4: Custom library with specific build options
ptah_add_library(
    NAME mylib
    GIT_REPOSITORY https://github.com/example/mylib.git
    VERSION v1.2.3
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
        -DCMAKE_CXX_STANDARD=17
)

# Save the manifest file
ptah_save_manifest(${CMAKE_CURRENT_SOURCE_DIR}/ptah-lock.yaml)

# Now you can use the libraries in your targets:
# find_package(SDL REQUIRED CONFIG PATHS ${PTAH_INSTALL_DIR})
# find_package(wgpu-native REQUIRED CONFIG PATHS ${PTAH_INSTALL_DIR})
# find_package(clay REQUIRED CONFIG PATHS ${PTAH_INSTALL_DIR})

# Or use the convenience function:
# ptah_find_library(SDL)
# ptah_find_library(wgpu-native)
# ptah_find_library(clay)

# Link to your targets:
# target_link_libraries(my_app PRIVATE
#     SDL::SDL
#     wgpu-native::wgpu-native
#     clay::clay
# )