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

# Include the Ptah Library Manager
include(${CMAKE_SOURCE_DIR}/cmake/PtahLibraryManager.cmake)

# Add SDL3 library using Ptah Library Manager
ptah_add_library(
    NAME SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    VERSION main
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DSDL_STATIC=ON
        -DSDL_SHARED=OFF
        -DSDL_TEST=OFF
        -DSDL_EXAMPLES=OFF
)

# Find the SDL3 library
ptah_find_library(SDL3)

# Create the SDL example executable
ptah_add_executable(sdl_example
    SOURCES
        sdl_example.c
    DEPENDENCIES
        SDL3::SDL3
)