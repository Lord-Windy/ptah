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

# Use FetchContent for Volk library
include(FetchContent)

FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk
    GIT_TAG master
)

# Set Volk build options
set(VOLK_INSTALL OFF CACHE BOOL "" FORCE)
set(VOLK_STATIC_DEFINES ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(volk)

# Find Vulkan SDK
find_package(Vulkan REQUIRED)

# Create the samvulkan library
ptah_add_library(samvulkan
    SOURCES
        src/samvulkan.c
        src/instance.c
        src/device.c
        src/compute.c
    PUBLIC_HEADERS
        include/samvulkan.h
        include/samvulkan/instance.h
        include/samvulkan/device.h
        include/samvulkan/compute.h
    DEPENDENCIES
        volk
        Vulkan::Vulkan
)

# Add tests if enabled
if(BUILD_TESTING)
    ptah_add_executable(samvulkan_test
        SOURCES
            test/test_samvulkan.c
        DEPENDENCIES
            samvulkan
    )
    add_test(NAME samvulkan_basic COMMAND samvulkan_test)
endif()