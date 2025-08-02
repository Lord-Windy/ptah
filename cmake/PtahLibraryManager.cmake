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

# Ptah Library Manager - Main CMake module for external library management
# Provides functionality to download, build, and integrate external libraries

include(CMakeParseArguments)
include(FetchContent)
include(ExternalProject)

# Global variables
set(PTAH_LIBRARY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external_dependencies")
set(PTAH_DOWNLOAD_DIR "${PTAH_LIBRARY_ROOT}/downloads")
set(PTAH_SOURCE_DIR "${PTAH_LIBRARY_ROOT}/sources")
set(PTAH_BUILD_DIR "${PTAH_LIBRARY_ROOT}/build")
set(PTAH_INSTALL_DIR "${PTAH_LIBRARY_ROOT}/install")

# Internal variables for manifest management
set(PTAH_MANIFEST_LOADED FALSE)
set(PTAH_MANIFEST_DATA "")

# Include all adapters
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahCMakeAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahCargoAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahMakeAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahMesonAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahHeaderOnlyAdapter.cmake)

# Main API functions
# ptah_add_library - Add and build an external library
function(ptah_add_library)
    set(options HEADER_ONLY INTERFACE_TARGET FORCE_REBUILD)
    set(oneValueArgs NAME GIT_REPOSITORY URL VERSION BUILD_SYSTEM)
    set(multiValueArgs CMAKE_ARGS BUILD_OPTIONS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Validate arguments
    if(NOT ARG_NAME)
        message(FATAL_ERROR "ptah_add_library: NAME is required")
    endif()
    
    if(NOT ARG_GIT_REPOSITORY AND NOT ARG_URL)
        message(FATAL_ERROR "ptah_add_library: GIT_REPOSITORY or URL is required")
    endif()
    
    # Set default version if not specified
    if(NOT ARG_VERSION)
        set(ARG_VERSION "main")
    endif()
    
    message(STATUS "Ptah: Adding library ${ARG_NAME}")
    
    # Load existing manifest
    ptah_load_manifest_internal()
    
    # Check if already built (unless FORCE_REBUILD)
    if(NOT ARG_FORCE_REBUILD)
        ptah_check_library_exists(${ARG_NAME} EXISTS)
        if(EXISTS)
            message(STATUS "Library ${ARG_NAME} already built, skipping...")
            return()
        endif()
    endif()
    
    # Resolve version to commit
    if(ARG_GIT_REPOSITORY)
        ptah_resolve_git_version(
            ${ARG_GIT_REPOSITORY} 
            ${ARG_VERSION}
            RESOLVED_COMMIT
            RESOLVED_REF
        )
        
        # Download/clone source
        ptah_fetch_source(
            ${ARG_NAME}
            ${ARG_GIT_REPOSITORY}
            ${RESOLVED_COMMIT}
            SOURCE_PATH
        )
    else()
        # Handle URL downloads (archives, etc.)
        ptah_fetch_archive(
            ${ARG_NAME}
            ${ARG_URL}
            SOURCE_PATH
        )
        set(RESOLVED_COMMIT "")
        set(RESOLVED_REF "")
    endif()
    
    # Auto-detect build system if not specified
    if(NOT ARG_BUILD_SYSTEM)
        ptah_detect_build_system(${SOURCE_PATH} DETECTED_SYSTEM)
        set(ARG_BUILD_SYSTEM ${DETECTED_SYSTEM})
    endif()
    
    message(STATUS "Ptah: Building ${ARG_NAME} using ${ARG_BUILD_SYSTEM}")
    
    # Build using appropriate adapter
    if(ARG_BUILD_SYSTEM STREQUAL "cmake")
        ptah_build_cmake_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR}
            CMAKE_ARGS ${ARG_CMAKE_ARGS})
    elseif(ARG_BUILD_SYSTEM STREQUAL "cargo")
        ptah_build_cargo_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR})
    elseif(ARG_BUILD_SYSTEM STREQUAL "make")
        ptah_build_make_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR}
            BUILD_OPTIONS ${ARG_BUILD_OPTIONS})
    elseif(ARG_BUILD_SYSTEM STREQUAL "meson")
        ptah_build_meson_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR})
    elseif(ARG_BUILD_SYSTEM STREQUAL "header_only" OR ARG_HEADER_ONLY)
        ptah_build_header_only_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR}
            INTERFACE_TARGET ${ARG_INTERFACE_TARGET})
    else()
        message(FATAL_ERROR "Unsupported build system: ${ARG_BUILD_SYSTEM}")
    endif()
    
    # Update manifest
    ptah_update_manifest(
        ${ARG_NAME}
        "${ARG_GIT_REPOSITORY}"
        "${ARG_VERSION}"
        "${RESOLVED_COMMIT}"
        "${RESOLVED_REF}"
        "${ARG_BUILD_SYSTEM}"
    )
    
    message(STATUS "Ptah: Successfully built and installed ${ARG_NAME}")
endfunction()

# ptah_find_library - Find and configure an external library for use
function(ptah_find_library NAME)
    set(CONFIG_PATH "${PTAH_INSTALL_DIR}/share/cmake/${NAME}")
    if(EXISTS "${CONFIG_PATH}/${NAME}Config.cmake")
        list(APPEND CMAKE_PREFIX_PATH ${PTAH_INSTALL_DIR})
        find_package(${NAME} REQUIRED CONFIG PATHS ${PTAH_INSTALL_DIR} NO_DEFAULT_PATH)
    else()
        message(WARNING "Ptah: No CMake config found for ${NAME}, you may need to set up find_package manually")
    endif()
endfunction()

# Internal functions for manifest management
function(ptah_load_manifest_internal)
    if(NOT PTAH_MANIFEST_LOADED)
        set(MANIFEST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ptah-lock.yaml")
        if(EXISTS ${MANIFEST_FILE})
            file(READ ${MANIFEST_FILE} PTAH_MANIFEST_DATA)
            set(PTAH_MANIFEST_DATA ${PTAH_MANIFEST_DATA} PARENT_SCOPE)
        endif()
        set(PTAH_MANIFEST_LOADED TRUE PARENT_SCOPE)
    endif()
endfunction()

# ptah_save_manifest - Save current manifest to file
function(ptah_save_manifest MANIFEST_PATH)
    if(NOT MANIFEST_PATH)
        set(MANIFEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ptah-lock.yaml")
    endif()
    
    # Generate YAML content
    string(TIMESTAMP CURRENT_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    set(YAML_CONTENT "version: 1.0\nlibraries:\n")
    
    # Write the manifest (this is a simplified version, full YAML generation would be more complex)
    file(WRITE ${MANIFEST_PATH} ${YAML_CONTENT})
    message(STATUS "Ptah: Saved manifest to ${MANIFEST_PATH}")
endfunction()

# Utility functions
function(ptah_check_library_exists NAME OUTPUT_VAR)
    set(INSTALL_MARKER "${PTAH_INSTALL_DIR}/.ptah/${NAME}.installed")
    if(EXISTS ${INSTALL_MARKER})
        set(${OUTPUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUTPUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(ptah_mark_library_installed NAME)
    file(MAKE_DIRECTORY "${PTAH_INSTALL_DIR}/.ptah")
    string(TIMESTAMP INSTALL_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    file(WRITE "${PTAH_INSTALL_DIR}/.ptah/${NAME}.installed" ${INSTALL_TIME})
endfunction()

function(ptah_detect_build_system SOURCE_DIR OUTPUT_VAR)
    if(EXISTS "${SOURCE_DIR}/CMakeLists.txt")
        set(${OUTPUT_VAR} "cmake" PARENT_SCOPE)
    elseif(EXISTS "${SOURCE_DIR}/Cargo.toml")
        set(${OUTPUT_VAR} "cargo" PARENT_SCOPE)
    elseif(EXISTS "${SOURCE_DIR}/meson.build")
        set(${OUTPUT_VAR} "meson" PARENT_SCOPE)
    elseif(EXISTS "${SOURCE_DIR}/Makefile" OR EXISTS "${SOURCE_DIR}/makefile")
        set(${OUTPUT_VAR} "make" PARENT_SCOPE)
    else()
        # Check for common header-only patterns
        file(GLOB_RECURSE HEADERS "${SOURCE_DIR}/*.h" "${SOURCE_DIR}/*.hpp")
        file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.c" "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.cc")
        
        list(LENGTH HEADERS HEADER_COUNT)
        list(LENGTH SOURCES SOURCE_COUNT)
        
        if(HEADER_COUNT GREATER 0 AND SOURCE_COUNT EQUAL 0)
            set(${OUTPUT_VAR} "header_only" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "Could not detect build system for ${SOURCE_DIR}")
        endif()
    endif()
endfunction()

function(ptah_resolve_git_version GIT_URL VERSION_SPEC OUTPUT_COMMIT OUTPUT_REF)
    # This is a simplified version - full implementation would use git ls-remote
    # For now, we'll just pass through the version as both commit and ref
    set(${OUTPUT_COMMIT} ${VERSION_SPEC} PARENT_SCOPE)
    set(${OUTPUT_REF} ${VERSION_SPEC} PARENT_SCOPE)
endfunction()

function(ptah_fetch_source NAME GIT_URL COMMIT_OR_TAG OUTPUT_PATH)
    set(SOURCE_PATH "${PTAH_SOURCE_DIR}/${NAME}")
    
    if(NOT EXISTS ${SOURCE_PATH})
        message(STATUS "Ptah: Cloning ${NAME} from ${GIT_URL}")
        execute_process(
            COMMAND git clone ${GIT_URL} ${SOURCE_PATH}
            RESULT_VARIABLE CLONE_RESULT
        )
        
        if(NOT CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone ${GIT_URL}")
        endif()
    endif()
    
    # Checkout specific commit/tag
    execute_process(
        COMMAND git checkout ${COMMIT_OR_TAG}
        WORKING_DIRECTORY ${SOURCE_PATH}
        RESULT_VARIABLE CHECKOUT_RESULT
    )
    
    if(NOT CHECKOUT_RESULT EQUAL 0)
        message(WARNING "Could not checkout ${COMMIT_OR_TAG}, using current HEAD")
    endif()
    
    set(${OUTPUT_PATH} ${SOURCE_PATH} PARENT_SCOPE)
endfunction()

function(ptah_fetch_archive NAME URL OUTPUT_PATH)
    # For future implementation - download and extract archives
    message(FATAL_ERROR "Archive downloads not yet implemented")
endfunction()

function(ptah_update_manifest NAME GIT_REPO VERSION COMMIT REF BUILD_SYSTEM)
    # For future implementation - update YAML manifest with library info
    message(STATUS "Ptah: Would update manifest for ${NAME}")
endfunction()

# Initialize Ptah system
function(ptah_initialize)
    message(STATUS "Ptah Library Manager initialized")
    message(STATUS "  Library root: ${PTAH_LIBRARY_ROOT}")
    message(STATUS "  Install directory: ${PTAH_INSTALL_DIR}")
    
    # Ensure directories exist
    file(MAKE_DIRECTORY ${PTAH_DOWNLOAD_DIR})
    file(MAKE_DIRECTORY ${PTAH_SOURCE_DIR})
    file(MAKE_DIRECTORY ${PTAH_BUILD_DIR})
    file(MAKE_DIRECTORY ${PTAH_INSTALL_DIR})
    
    # Add install directory to CMAKE_PREFIX_PATH
    list(APPEND CMAKE_PREFIX_PATH ${PTAH_INSTALL_DIR})
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
endfunction()

# Auto-initialize when module is included
ptah_initialize()