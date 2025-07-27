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

cmake_minimum_required(VERSION 3.16)
project(ptah VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler flags for Valgrind compatibility in Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Disable advanced instruction sets that Valgrind might not support
    # Use a more conservative architecture setting
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -march=x86-64 -mtune=generic -mno-avx -mno-avx2 -mno-avx512f -mno-avx512dq -mno-avx512cd -mno-avx512bw -mno-avx512vl -mno-avx512ifma -mno-avx512vbmi")
endif()

# Export compile commands for editor integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "Valgrind")
endif()

# Custom Valgrind build type with restricted instruction sets
set(CMAKE_C_FLAGS_VALGRIND "-O0 -g -march=x86-64 -mtune=generic -mno-avx -mno-avx2 -mno-avx512f -mno-avx512dq -mno-avx512cd -mno-avx512bw -mno-avx512vl -mno-avx512ifma -mno-avx512vbmi -mno-sse4.2 -mno-popcnt -mno-lzcnt -mno-bmi -mno-bmi2" CACHE STRING
    "Flags used by the C compiler during Valgrind builds." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_VALGRIND "-static-libgcc" CACHE STRING
    "Flags used for linking binaries during Valgrind builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_VALGRIND "" CACHE STRING
    "Flags used by the shared libraries linker during Valgrind builds." FORCE)
mark_as_advanced(
    CMAKE_C_FLAGS_VALGRIND
    CMAKE_EXE_LINKER_FLAGS_VALGRIND
    CMAKE_SHARED_LINKER_FLAGS_VALGRIND)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTING "Build tests" ON)

# Include useful modules
include(CTest)
include(GNUInstallDirs)

# Helper function for creating libraries with consistent settings
function(ptah_add_library name)
    set(options STATIC SHARED INTERFACE)
    set(oneValueArgs)
    set(multiValueArgs SOURCES PUBLIC_HEADERS DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(ARG_INTERFACE)
        add_library(${name} INTERFACE)
    elseif(ARG_STATIC)
        add_library(${name} STATIC ${ARG_SOURCES})
    elseif(ARG_SHARED)
        add_library(${name} SHARED ${ARG_SOURCES})
    else()
        add_library(${name} ${ARG_SOURCES})
    endif()
    
    if(NOT ARG_INTERFACE)
        target_include_directories(${name}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )
    endif()
    
    if(ARG_DEPENDENCIES)
        target_link_libraries(${name} PUBLIC ${ARG_DEPENDENCIES})
    endif()
    
    # Install rules
    if(ARG_PUBLIC_HEADERS)
        set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${ARG_PUBLIC_HEADERS}")
    endif()
    
    install(TARGETS ${name}
        EXPORT ptahTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${name}
    )
endfunction()

# Helper function for creating executables with consistent settings
function(ptah_add_executable name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    add_executable(${name} ${ARG_SOURCES})
    
    if(ARG_DEPENDENCIES)
        target_link_libraries(${name} PRIVATE ${ARG_DEPENDENCIES})
    endif()
    
    install(TARGETS ${name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endfunction()

# Function to recursively find and add subdirectories with CMakeLists.txt
function(add_subdirectories_recursively dir)
    file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${dir} ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${child})
            if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${child}/CMakeLists.txt)
                add_subdirectory(${dir}/${child})
            else()
                add_subdirectories_recursively(${dir}/${child})
            endif()
        endif()
    endforeach()
endfunction()

# Automatically discover and add all subdirectories with CMakeLists.txt
# This allows for a Bazel-like structure where each library/app has its own CMakeLists.txt
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
    add_subdirectories_recursively(libs)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/apps)
    add_subdirectories_recursively(apps)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    add_subdirectories_recursively(tools)
endif()

# Custom targets
# Linting target for C code
add_custom_target(lint
    COMMAND find ${CMAKE_SOURCE_DIR} -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Linting C code with clang-format"
    VERBATIM
)

add_custom_target(format
    COMMAND find ${CMAKE_SOURCE_DIR} -name "*.c" -o -name "*.h" | xargs clang-format -i
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Formatting C code with clang-format"
    VERBATIM
)

# Testing
if(BUILD_TESTING)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
        add_subdirectories_recursively(tests)
    endif()
endif()