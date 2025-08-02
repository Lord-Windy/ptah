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
set(PTAH_MANIFEST_LIBRARIES "")

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
    
    # Check if already built with correct version (unless FORCE_REBUILD)
    if(NOT ARG_FORCE_REBUILD)
        ptah_check_library_exists(${ARG_NAME} EXISTS)
        if(EXISTS)
            # Check if version matches
            ptah_check_version_match(${ARG_NAME} ${ARG_VERSION} VERSION_MATCHES)
            if(VERSION_MATCHES)
                message(STATUS "Library ${ARG_NAME} already built with correct version, skipping...")
                return()
            else()
                message(STATUS "Library ${ARG_NAME} exists but version differs, rebuilding...")
            endif()
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
    
    # Mark library as installed
    ptah_mark_library_installed(${ARG_NAME})
    
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
            
            # Parse the YAML content
            ptah_parse_manifest("${PTAH_MANIFEST_DATA}" PARSED_LIBRARIES)
            set(PTAH_MANIFEST_LIBRARIES ${PARSED_LIBRARIES} PARENT_SCOPE)
        endif()
        set(PTAH_MANIFEST_LOADED TRUE PARENT_SCOPE)
    endif()
endfunction()

# Parse YAML manifest content (simplified parser for our specific format)
function(ptah_parse_manifest YAML_CONTENT OUTPUT_LIBRARIES)
    set(LIBRARIES "")
    
    # Split into lines
    string(REPLACE "\n" ";" LINES "${YAML_CONTENT}")
    
    set(CURRENT_LIBRARY "")
    set(IN_LIBRARIES_SECTION FALSE)
    
    foreach(LINE ${LINES})
        # Remove leading/trailing whitespace
        string(STRIP "${LINE}" STRIPPED_LINE)
        
        # Skip empty lines and comments
        if(NOT STRIPPED_LINE OR STRIPPED_LINE MATCHES "^#")
            continue()
        endif()
        
        # Check for libraries section
        if(STRIPPED_LINE STREQUAL "libraries:")
            set(IN_LIBRARIES_SECTION TRUE)
            continue()
        endif()
        
        if(IN_LIBRARIES_SECTION)
            # Check for library name (no leading spaces, ends with colon)
            if(STRIPPED_LINE MATCHES "^([a-zA-Z0-9_-]+):$")
                set(CURRENT_LIBRARY ${CMAKE_MATCH_1})
                list(APPEND LIBRARIES ${CURRENT_LIBRARY})
                
                # Initialize library data
                set(PTAH_LIB_${CURRENT_LIBRARY}_SOURCE "" PARENT_SCOPE)
                set(PTAH_LIB_${CURRENT_LIBRARY}_VERSION_SPEC "" PARENT_SCOPE)
                set(PTAH_LIB_${CURRENT_LIBRARY}_RESOLVED_TAG "" PARENT_SCOPE)
                set(PTAH_LIB_${CURRENT_LIBRARY}_RESOLVED_REF "" PARENT_SCOPE)
                set(PTAH_LIB_${CURRENT_LIBRARY}_COMMIT "" PARENT_SCOPE)
                set(PTAH_LIB_${CURRENT_LIBRARY}_BUILD_SYSTEM "" PARENT_SCOPE)
                set(PTAH_LIB_${CURRENT_LIBRARY}_LAST_UPDATED "" PARENT_SCOPE)
                continue()
            endif()
            
            # Parse library properties (indented lines)
            if(CURRENT_LIBRARY AND STRIPPED_LINE MATCHES "^([a-zA-Z_]+): *(.+)$")
                set(PROP_NAME ${CMAKE_MATCH_1})
                set(PROP_VALUE ${CMAKE_MATCH_2})
                
                # Remove quotes if present
                string(REGEX REPLACE "^\"(.*)\"$" "\\1" PROP_VALUE ${PROP_VALUE})
                
                # Set the property
                if(PROP_NAME STREQUAL "source")
                    set(PTAH_LIB_${CURRENT_LIBRARY}_SOURCE ${PROP_VALUE} PARENT_SCOPE)
                elseif(PROP_NAME STREQUAL "version_spec")
                    set(PTAH_LIB_${CURRENT_LIBRARY}_VERSION_SPEC ${PROP_VALUE} PARENT_SCOPE)
                elseif(PROP_NAME STREQUAL "resolved_tag")
                    set(PTAH_LIB_${CURRENT_LIBRARY}_RESOLVED_TAG ${PROP_VALUE} PARENT_SCOPE)
                elseif(PROP_NAME STREQUAL "resolved_ref")
                    set(PTAH_LIB_${CURRENT_LIBRARY}_RESOLVED_REF ${PROP_VALUE} PARENT_SCOPE)
                elseif(PROP_NAME STREQUAL "commit")
                    set(PTAH_LIB_${CURRENT_LIBRARY}_COMMIT ${PROP_VALUE} PARENT_SCOPE)
                elseif(PROP_NAME STREQUAL "build_system")
                    set(PTAH_LIB_${CURRENT_LIBRARY}_BUILD_SYSTEM ${PROP_VALUE} PARENT_SCOPE)
                elseif(PROP_NAME STREQUAL "last_updated")
                    set(PTAH_LIB_${CURRENT_LIBRARY}_LAST_UPDATED ${PROP_VALUE} PARENT_SCOPE)
                endif()
            endif()
        endif()
    endforeach()
    
    set(${OUTPUT_LIBRARIES} ${LIBRARIES} PARENT_SCOPE)
endfunction()

# Generate YAML manifest content
function(ptah_generate_manifest_yaml OUTPUT_CONTENT)
    string(TIMESTAMP CURRENT_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    set(YAML_CONTENT "version: 1.0\nlibraries:\n")
    
    # Iterate through all libraries in the manifest
    foreach(LIB_NAME ${PTAH_MANIFEST_LIBRARIES})
        string(APPEND YAML_CONTENT "  ${LIB_NAME}:\n")
        
        if(PTAH_LIB_${LIB_NAME}_SOURCE)
            string(APPEND YAML_CONTENT "    source: \"${PTAH_LIB_${LIB_NAME}_SOURCE}\"\n")
        endif()
        
        if(PTAH_LIB_${LIB_NAME}_VERSION_SPEC)
            string(APPEND YAML_CONTENT "    version_spec: \"${PTAH_LIB_${LIB_NAME}_VERSION_SPEC}\"\n")
        endif()
        
        if(PTAH_LIB_${LIB_NAME}_RESOLVED_TAG)
            string(APPEND YAML_CONTENT "    resolved_tag: \"${PTAH_LIB_${LIB_NAME}_RESOLVED_TAG}\"\n")
        endif()
        
        if(PTAH_LIB_${LIB_NAME}_RESOLVED_REF)
            string(APPEND YAML_CONTENT "    resolved_ref: \"${PTAH_LIB_${LIB_NAME}_RESOLVED_REF}\"\n")
        endif()
        
        if(PTAH_LIB_${LIB_NAME}_COMMIT)
            string(APPEND YAML_CONTENT "    commit: \"${PTAH_LIB_${LIB_NAME}_COMMIT}\"\n")
        endif()
        
        if(PTAH_LIB_${LIB_NAME}_BUILD_SYSTEM)
            string(APPEND YAML_CONTENT "    build_system: ${PTAH_LIB_${LIB_NAME}_BUILD_SYSTEM}\n")
        endif()
        
        if(PTAH_LIB_${LIB_NAME}_LAST_UPDATED)
            string(APPEND YAML_CONTENT "    last_updated: \"${PTAH_LIB_${LIB_NAME}_LAST_UPDATED}\"\n")
        endif()
        
        string(APPEND YAML_CONTENT "\n")
    endforeach()
    
    set(${OUTPUT_CONTENT} ${YAML_CONTENT} PARENT_SCOPE)
endfunction()

# Check if library exists in manifest
function(ptah_library_in_manifest LIB_NAME OUTPUT_VAR)
    list(FIND PTAH_MANIFEST_LIBRARIES ${LIB_NAME} FOUND_INDEX)
    if(FOUND_INDEX GREATER_EQUAL 0)
        set(${OUTPUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUTPUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Add or update library in manifest
function(ptah_add_library_to_manifest LIB_NAME SOURCE VERSION_SPEC COMMIT RESOLVED_REF BUILD_SYSTEM)
    string(TIMESTAMP CURRENT_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    # Check if library already exists in the list
    ptah_library_in_manifest(${LIB_NAME} EXISTS)
    if(NOT EXISTS)
        list(APPEND PTAH_MANIFEST_LIBRARIES ${LIB_NAME})
        set(PTAH_MANIFEST_LIBRARIES ${PTAH_MANIFEST_LIBRARIES} PARENT_SCOPE)
    endif()
    
    # Set library properties
    set(PTAH_LIB_${LIB_NAME}_SOURCE ${SOURCE} PARENT_SCOPE)
    set(PTAH_LIB_${LIB_NAME}_VERSION_SPEC ${VERSION_SPEC} PARENT_SCOPE)
    set(PTAH_LIB_${LIB_NAME}_COMMIT ${COMMIT} PARENT_SCOPE)
    set(PTAH_LIB_${LIB_NAME}_RESOLVED_REF ${RESOLVED_REF} PARENT_SCOPE)
    set(PTAH_LIB_${LIB_NAME}_BUILD_SYSTEM ${BUILD_SYSTEM} PARENT_SCOPE)
    set(PTAH_LIB_${LIB_NAME}_LAST_UPDATED ${CURRENT_TIME} PARENT_SCOPE)
    
    # Determine resolved tag vs resolved ref
    if(RESOLVED_REF MATCHES "^v?[0-9]+\\.[0-9]+")
        set(PTAH_LIB_${LIB_NAME}_RESOLVED_TAG ${RESOLVED_REF} PARENT_SCOPE)
        set(PTAH_LIB_${LIB_NAME}_RESOLVED_REF "" PARENT_SCOPE)
    else()
        set(PTAH_LIB_${LIB_NAME}_RESOLVED_TAG "" PARENT_SCOPE)
        set(PTAH_LIB_${LIB_NAME}_RESOLVED_REF ${RESOLVED_REF} PARENT_SCOPE)
    endif()
endfunction()

# ptah_save_manifest - Save current manifest to file
function(ptah_save_manifest MANIFEST_PATH)
    if(NOT MANIFEST_PATH)
        set(MANIFEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ptah-lock.yaml")
    endif()
    
    # Generate YAML content from current manifest data
    ptah_generate_manifest_yaml(YAML_CONTENT)
    
    # Write the manifest
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

# Check if requested version matches what's already built
function(ptah_check_version_match NAME REQUESTED_VERSION OUTPUT_VAR)
    ptah_library_in_manifest(${NAME} IN_MANIFEST)
    if(IN_MANIFEST)
        # Compare requested version with manifest version
        if(PTAH_LIB_${NAME}_VERSION_SPEC STREQUAL ${REQUESTED_VERSION})
            set(${OUTPUT_VAR} TRUE PARENT_SCOPE)
        else()
            message(STATUS "Ptah: Version mismatch for ${NAME}: requested '${REQUESTED_VERSION}', manifest has '${PTAH_LIB_${NAME}_VERSION_SPEC}'")
            set(${OUTPUT_VAR} FALSE PARENT_SCOPE)
        endif()
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
    message(STATUS "Ptah: Resolving version '${VERSION_SPEC}' for ${GIT_URL}")
    
    # Initialize outputs
    set(RESOLVED_COMMIT "")
    set(RESOLVED_REF "")
    
    # First, check if VERSION_SPEC is already a full commit hash (40 chars)
    string(LENGTH ${VERSION_SPEC} VERSION_LENGTH)
    if(VERSION_LENGTH EQUAL 40)
        string(REGEX MATCH "^[a-fA-F0-9]+$" IS_COMMIT ${VERSION_SPEC})
        if(IS_COMMIT)
            set(RESOLVED_COMMIT ${VERSION_SPEC})
            set(RESOLVED_REF ${VERSION_SPEC})
            set(${OUTPUT_COMMIT} ${RESOLVED_COMMIT} PARENT_SCOPE)
            set(${OUTPUT_REF} ${RESOLVED_REF} PARENT_SCOPE)
            return()
        endif()
    endif()
    
    # Use git ls-remote to get all refs
    execute_process(
        COMMAND git ls-remote --heads --tags ${GIT_URL}
        OUTPUT_VARIABLE GIT_REFS
        ERROR_VARIABLE GIT_ERROR
        RESULT_VARIABLE GIT_RESULT
    )
    
    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to query Git repository ${GIT_URL}: ${GIT_ERROR}")
    endif()
    
    # Parse refs and look for matches
    string(REPLACE "\n" ";" REF_LINES "${GIT_REFS}")
    
    # Strategy 1: Look for exact tag match (e.g., "v3.0.0" or "3.0.0")
    foreach(LINE ${REF_LINES})
        string(REGEX MATCH "^([a-fA-F0-9]+)[\t ]+refs/tags/(.+)$" TAG_MATCH ${LINE})
        if(TAG_MATCH)
            set(COMMIT ${CMAKE_MATCH_1})
            set(TAG ${CMAKE_MATCH_2})
            
            # Check for exact match
            if(TAG STREQUAL ${VERSION_SPEC})
                set(RESOLVED_COMMIT ${COMMIT})
                set(RESOLVED_REF ${TAG})
                break()
            endif()
            
            # Check for match without 'v' prefix
            if(VERSION_SPEC MATCHES "^v?(.+)$")
                set(VERSION_WITHOUT_V ${CMAKE_MATCH_1})
                if(TAG STREQUAL "v${VERSION_WITHOUT_V}" OR TAG STREQUAL ${VERSION_WITHOUT_V})
                    set(RESOLVED_COMMIT ${COMMIT})
                    set(RESOLVED_REF ${TAG})
                    break()
                endif()
            endif()
        endif()
    endforeach()
    
    # Strategy 2: If no tag found, look for branch match
    if(NOT RESOLVED_COMMIT)
        foreach(LINE ${REF_LINES})
            string(REGEX MATCH "^([a-fA-F0-9]+)[\t ]+refs/heads/(.+)$" BRANCH_MATCH ${LINE})
            if(BRANCH_MATCH)
                set(COMMIT ${CMAKE_MATCH_1})
                set(BRANCH ${CMAKE_MATCH_2})
                
                if(BRANCH STREQUAL ${VERSION_SPEC})
                    set(RESOLVED_COMMIT ${COMMIT})
                    set(RESOLVED_REF ${BRANCH})
                    break()
                endif()
            endif()
        endforeach()
    endif()
    
    # Strategy 3: Try partial commit hash match
    if(NOT RESOLVED_COMMIT AND VERSION_LENGTH GREATER_EQUAL 7)
        string(REGEX MATCH "^[a-fA-F0-9]+$" IS_PARTIAL_COMMIT ${VERSION_SPEC})
        if(IS_PARTIAL_COMMIT)
            foreach(LINE ${REF_LINES})
                string(REGEX MATCH "^([a-fA-F0-9]+)" COMMIT_FROM_LINE ${LINE})
                if(COMMIT_FROM_LINE)
                    string(SUBSTRING ${CMAKE_MATCH_1} 0 ${VERSION_LENGTH} COMMIT_PREFIX)
                    if(COMMIT_PREFIX STREQUAL ${VERSION_SPEC})
                        set(RESOLVED_COMMIT ${CMAKE_MATCH_1})
                        set(RESOLVED_REF ${VERSION_SPEC})
                        break()
                    endif()
                endif()
            endforeach()
        endif()
    endif()
    
    # If still not found, error out
    if(NOT RESOLVED_COMMIT)
        message(FATAL_ERROR "Could not resolve version '${VERSION_SPEC}' in repository ${GIT_URL}")
    endif()
    
    message(STATUS "Ptah: Resolved '${VERSION_SPEC}' to commit ${RESOLVED_COMMIT} (ref: ${RESOLVED_REF})")
    
    set(${OUTPUT_COMMIT} ${RESOLVED_COMMIT} PARENT_SCOPE)
    set(${OUTPUT_REF} ${RESOLVED_REF} PARENT_SCOPE)
endfunction()

function(ptah_fetch_source NAME GIT_URL COMMIT_OR_TAG OUTPUT_PATH)
    set(SOURCE_PATH "${PTAH_SOURCE_DIR}/${NAME}")
    
    if(NOT EXISTS ${SOURCE_PATH})
        message(STATUS "Ptah: Cloning ${NAME} from ${GIT_URL}")
        execute_process(
            COMMAND git clone ${GIT_URL} ${SOURCE_PATH}
            RESULT_VARIABLE CLONE_RESULT
            OUTPUT_QUIET
            ERROR_QUIET
        )
        
        if(NOT CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone ${GIT_URL}")
        endif()
    else()
        # If directory exists, fetch latest refs
        message(STATUS "Ptah: Updating existing repository for ${NAME}")
        execute_process(
            COMMAND git fetch origin
            WORKING_DIRECTORY ${SOURCE_PATH}
            OUTPUT_QUIET
            ERROR_QUIET
        )
    endif()
    
    # Checkout specific commit (this ensures we get exactly what we resolved)
    message(STATUS "Ptah: Checking out commit ${COMMIT_OR_TAG}")
    execute_process(
        COMMAND git checkout ${COMMIT_OR_TAG}
        WORKING_DIRECTORY ${SOURCE_PATH}
        RESULT_VARIABLE CHECKOUT_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT CHECKOUT_RESULT EQUAL 0)
        message(FATAL_ERROR "Could not checkout ${COMMIT_OR_TAG} in ${SOURCE_PATH}")
    endif()
    
    # Verify we're on the correct commit
    execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${SOURCE_PATH}
        OUTPUT_VARIABLE CURRENT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(NOT CURRENT_COMMIT STREQUAL COMMIT_OR_TAG)
        message(WARNING "Ptah: Expected commit ${COMMIT_OR_TAG} but got ${CURRENT_COMMIT}")
    endif()
    
    set(${OUTPUT_PATH} ${SOURCE_PATH} PARENT_SCOPE)
endfunction()

function(ptah_fetch_archive NAME URL OUTPUT_PATH)
    # For future implementation - download and extract archives
    message(FATAL_ERROR "Archive downloads not yet implemented")
endfunction()

function(ptah_update_manifest NAME GIT_REPO VERSION COMMIT REF BUILD_SYSTEM)
    # Add or update library in the manifest
    ptah_add_library_to_manifest(${NAME} "${GIT_REPO}" "${VERSION}" "${COMMIT}" "${REF}" "${BUILD_SYSTEM}")
    message(STATUS "Ptah: Updated manifest for ${NAME}")
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