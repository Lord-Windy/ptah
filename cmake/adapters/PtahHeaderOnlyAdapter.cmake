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

# Header-Only Library Adapter for Ptah Library Manager
# Handles header-only libraries (e.g., Clay, single-header libraries)

function(ptah_build_header_only_library NAME SOURCE_DIR INSTALL_DIR)
    set(options INTERFACE_TARGET SINGLE_HEADER)
    set(oneValueArgs HEADER_DIR NAMESPACE)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "" ${ARGN})
    
    message(STATUS "Ptah Header-Only: Installing ${NAME}")
    
    # Default header directory
    if(NOT ARG_HEADER_DIR)
        set(ARG_HEADER_DIR ".")
    endif()
    
    # Install headers
    ptah_header_only_install_headers(${NAME} ${SOURCE_DIR} ${INSTALL_DIR} ${ARG_HEADER_DIR} ${ARG_SINGLE_HEADER})
    
    # Generate CMake config
    if(ARG_INTERFACE_TARGET)
        ptah_header_only_generate_interface_config(${NAME} ${INSTALL_DIR} ${ARG_NAMESPACE})
    else()
        ptah_header_only_generate_basic_config(${NAME} ${INSTALL_DIR})
    endif()
    
    # Mark as installed
    ptah_mark_library_installed(${NAME})
    
    message(STATUS "Ptah Header-Only: Successfully installed ${NAME}")
endfunction()

function(ptah_header_only_install_headers NAME SOURCE_DIR INSTALL_DIR HEADER_DIR SINGLE_HEADER)
    set(SOURCE_HEADER_DIR "${SOURCE_DIR}/${HEADER_DIR}")
    
    if(SINGLE_HEADER)
        # For single-header libraries, find the main header
        ptah_header_only_find_main_header(${SOURCE_HEADER_DIR} MAIN_HEADER)
        if(MAIN_HEADER)
            file(MAKE_DIRECTORY "${INSTALL_DIR}/include")
            file(COPY ${MAIN_HEADER} DESTINATION "${INSTALL_DIR}/include")
            get_filename_component(HEADER_NAME ${MAIN_HEADER} NAME)
            message(STATUS "Ptah Header-Only: Installed single header ${HEADER_NAME}")
        else()
            message(FATAL_ERROR "Ptah Header-Only: Could not find main header in ${SOURCE_HEADER_DIR}")
        endif()
    else()
        # Install all headers, preserving directory structure
        if(EXISTS ${SOURCE_HEADER_DIR})
            # Find all header files
            file(GLOB_RECURSE HEADERS 
                "${SOURCE_HEADER_DIR}/*.h"
                "${SOURCE_HEADER_DIR}/*.hpp"
                "${SOURCE_HEADER_DIR}/*.hxx"
                "${SOURCE_HEADER_DIR}/*.hh"
            )
            
            if(HEADERS)
                # Create target include directory
                file(MAKE_DIRECTORY "${INSTALL_DIR}/include/${NAME}")
                
                # Copy headers maintaining structure
                foreach(HEADER ${HEADERS})
                    file(RELATIVE_PATH REL_PATH ${SOURCE_HEADER_DIR} ${HEADER})
                    get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
                    if(REL_DIR)
                        file(MAKE_DIRECTORY "${INSTALL_DIR}/include/${NAME}/${REL_DIR}")
                        file(COPY ${HEADER} DESTINATION "${INSTALL_DIR}/include/${NAME}/${REL_DIR}")
                    else()
                        file(COPY ${HEADER} DESTINATION "${INSTALL_DIR}/include/${NAME}")
                    endif()
                endforeach()
                
                list(LENGTH HEADERS HEADER_COUNT)
                message(STATUS "Ptah Header-Only: Installed ${HEADER_COUNT} headers")
            else()
                message(FATAL_ERROR "Ptah Header-Only: No headers found in ${SOURCE_HEADER_DIR}")
            endif()
        else()
            message(FATAL_ERROR "Ptah Header-Only: Header directory ${SOURCE_HEADER_DIR} does not exist")
        endif()
    endif()
endfunction()

function(ptah_header_only_find_main_header SOURCE_DIR OUTPUT_VAR)
    # Look for common single-header library patterns
    set(CANDIDATES
        "${SOURCE_DIR}/${NAME}.h"
        "${SOURCE_DIR}/${NAME}.hpp"
    )
    
    # Look for headers with the same name as the directory
    get_filename_component(DIR_NAME ${SOURCE_DIR} NAME)
    list(APPEND CANDIDATES
        "${SOURCE_DIR}/${DIR_NAME}.h"
        "${SOURCE_DIR}/${DIR_NAME}.hpp"
    )
    
    # Look for common single-header names
    file(GLOB POTENTIAL_HEADERS 
        "${SOURCE_DIR}/*.h"
        "${SOURCE_DIR}/*.hpp"
    )
    
    # Prefer files that have "header-only" characteristics (larger size)
    set(BEST_HEADER "")
    set(BEST_SIZE 0)
    
    foreach(HEADER ${POTENTIAL_HEADERS})
        file(SIZE ${HEADER} HEADER_SIZE)
        if(HEADER_SIZE GREATER BEST_SIZE)
            set(BEST_HEADER ${HEADER})
            set(BEST_SIZE ${HEADER_SIZE})
        endif()
    endforeach()
    
    # Check candidates first
    foreach(CANDIDATE ${CANDIDATES})
        if(EXISTS ${CANDIDATE})
            set(${OUTPUT_VAR} ${CANDIDATE} PARENT_SCOPE)
            return()
        endif()
    endforeach()
    
    # Fall back to largest header
    if(BEST_HEADER)
        set(${OUTPUT_VAR} ${BEST_HEADER} PARENT_SCOPE)
    else()
        set(${OUTPUT_VAR} "" PARENT_SCOPE)
    endif()
endfunction()

function(ptah_header_only_generate_interface_config NAME INSTALL_DIR NAMESPACE)
    set(CONFIG_DIR "${INSTALL_DIR}/share/cmake/${NAME}")
    set(CONFIG_FILE "${CONFIG_DIR}/${NAME}Config.cmake")
    
    file(MAKE_DIRECTORY ${CONFIG_DIR})
    
    # Use namespace if provided, otherwise use library name
    if(NOT NAMESPACE)
        set(NAMESPACE ${NAME})
    endif()
    
    # Generate interface library config
    set(CONFIG_CONTENT "# Generated by Ptah Library Manager
# CMake configuration for ${NAME} (Header-only library)

include(CMakeFindDependencyMacro)

# Define the interface target
if(NOT TARGET ${NAMESPACE}::${NAME})
    add_library(${NAMESPACE}::${NAME} INTERFACE IMPORTED)
    
    # Set include directories
    set_target_properties(${NAMESPACE}::${NAME} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES \"${INSTALL_DIR}/include\"
    )
    
    # Add compile features if this is a C++ library
    file(GLOB_RECURSE CPP_HEADERS \"${INSTALL_DIR}/include/*.hpp\" \"${INSTALL_DIR}/include/*.hxx\")
    if(CPP_HEADERS)
        set_target_properties(${NAMESPACE}::${NAME} PROPERTIES
            INTERFACE_COMPILE_FEATURES cxx_std_11
        )
    endif()
endif()

# Legacy target name for compatibility
if(NOT TARGET ${NAME}::${NAME} AND NOT \"${NAMESPACE}\" STREQUAL \"${NAME}\")
    add_library(${NAME}::${NAME} ALIAS ${NAMESPACE}::${NAME})
endif()

# Mark as found
set(${NAME}_FOUND TRUE)
set(${NAME}_INCLUDE_DIRS \"${INSTALL_DIR}/include\")
")
    
    file(WRITE ${CONFIG_FILE} ${CONFIG_CONTENT})
    
    # Generate version file
    set(VERSION_FILE "${CONFIG_DIR}/${NAME}ConfigVersion.cmake")
    set(VERSION_CONTENT "# Generated by Ptah Library Manager
# Version information for ${NAME} (Header-only library)

set(PACKAGE_VERSION \"unknown\")
set(PACKAGE_VERSION_COMPATIBLE TRUE)
set(PACKAGE_VERSION_EXACT FALSE)
")
    file(WRITE ${VERSION_FILE} ${VERSION_CONTENT})
endfunction()

function(ptah_header_only_generate_basic_config NAME INSTALL_DIR)
    set(CONFIG_DIR "${INSTALL_DIR}/share/cmake/${NAME}")
    set(CONFIG_FILE "${CONFIG_DIR}/${NAME}Config.cmake")
    
    file(MAKE_DIRECTORY ${CONFIG_DIR})
    
    # Generate basic config (just sets variables)
    set(CONFIG_CONTENT "# Generated by Ptah Library Manager
# CMake configuration for ${NAME} (Header-only library)

# Set include directories
set(${NAME}_INCLUDE_DIRS \"${INSTALL_DIR}/include\")
set(${NAME}_FOUND TRUE)

# For compatibility with different naming conventions
set(${NAME}_INCLUDE_DIR \"\${${NAME}_INCLUDE_DIRS}\")
")
    
    file(WRITE ${CONFIG_FILE} ${CONFIG_CONTENT})
    
    # Generate version file
    set(VERSION_FILE "${CONFIG_DIR}/${NAME}ConfigVersion.cmake")
    set(VERSION_CONTENT "# Generated by Ptah Library Manager
set(PACKAGE_VERSION \"unknown\")
set(PACKAGE_VERSION_COMPATIBLE TRUE)
set(PACKAGE_VERSION_EXACT FALSE)
")
    file(WRITE ${VERSION_FILE} ${VERSION_CONTENT})
endfunction()

# Utility function to detect header-only library characteristics
function(ptah_header_only_detect_type SOURCE_DIR OUTPUT_TYPE OUTPUT_MAIN_HEADER)
    # Check if it's a single-header library
    file(GLOB ROOT_HEADERS "${SOURCE_DIR}/*.h" "${SOURCE_DIR}/*.hpp")
    list(LENGTH ROOT_HEADERS ROOT_HEADER_COUNT)
    
    if(ROOT_HEADER_COUNT EQUAL 1)
        list(GET ROOT_HEADERS 0 MAIN_HEADER)
        file(SIZE ${MAIN_HEADER} HEADER_SIZE)
        
        # If the single header is large (>10KB), it's likely a single-header library
        if(HEADER_SIZE GREATER 10240)
            set(${OUTPUT_TYPE} "SINGLE_HEADER" PARENT_SCOPE)
            set(${OUTPUT_MAIN_HEADER} ${MAIN_HEADER} PARENT_SCOPE)
            return()
        endif()
    endif()
    
    # Check for common single-header library indicators
    foreach(HEADER ${ROOT_HEADERS})
        file(READ ${HEADER} HEADER_CONTENT)
        # Look for single-header library patterns
        if(HEADER_CONTENT MATCHES "#ifndef.*IMPLEMENTATION" OR 
           HEADER_CONTENT MATCHES "#ifdef.*IMPLEMENTATION" OR
           HEADER_CONTENT MATCHES "header.?only" OR
           HEADER_CONTENT MATCHES "single.?header")
            set(${OUTPUT_TYPE} "SINGLE_HEADER" PARENT_SCOPE)
            set(${OUTPUT_MAIN_HEADER} ${HEADER} PARENT_SCOPE)
            return()
        endif()
    endforeach()
    
    # Default to multi-header
    set(${OUTPUT_TYPE} "MULTI_HEADER" PARENT_SCOPE)
    set(${OUTPUT_MAIN_HEADER} "" PARENT_SCOPE)
endfunction()