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

# CMake Build System Adapter for Ptah Library Manager
# Handles building CMake-based external libraries (e.g., SDL)

function(ptah_build_cmake_library NAME SOURCE_DIR INSTALL_DIR)
    set(multiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
    
    set(BUILD_DIR "${PTAH_BUILD_DIR}/${NAME}")
    
    message(STATUS "Ptah CMake: Configuring ${NAME}")
    
    # Prepare CMake arguments
    set(CMAKE_CONFIGURE_ARGS
        -S "${SOURCE_DIR}"
        -B "${BUILD_DIR}"
        -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )
    
    # Add any additional CMake args
    if(ARG_CMAKE_ARGS)
        list(APPEND CMAKE_CONFIGURE_ARGS ${ARG_CMAKE_ARGS})
    endif()
    
    # Common CMake arguments for better compatibility
    list(APPEND CMAKE_CONFIGURE_ARGS
        -DBUILD_SHARED_LIBS=OFF  # Prefer static libs for easier distribution
        -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON
    )
    
    # Configure step
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${CMAKE_CONFIGURE_ARGS}
        RESULT_VARIABLE CONFIG_RESULT
        OUTPUT_VARIABLE CONFIG_OUTPUT
        ERROR_VARIABLE CONFIG_ERROR
    )
    
    if(NOT CONFIG_RESULT EQUAL 0)
        message(FATAL_ERROR "Ptah CMake: Configuration failed for ${NAME}:\n${CONFIG_ERROR}")
    endif()
    
    message(STATUS "Ptah CMake: Building ${NAME}")
    
    # Build step
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build "${BUILD_DIR}" --parallel
        RESULT_VARIABLE BUILD_RESULT
        OUTPUT_VARIABLE BUILD_OUTPUT
        ERROR_VARIABLE BUILD_ERROR
    )
    
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Ptah CMake: Build failed for ${NAME}:\n${BUILD_ERROR}")
    endif()
    
    message(STATUS "Ptah CMake: Installing ${NAME}")
    
    # Install step
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install "${BUILD_DIR}"
        RESULT_VARIABLE INSTALL_RESULT
        OUTPUT_VARIABLE INSTALL_OUTPUT
        ERROR_VARIABLE INSTALL_ERROR
    )
    
    if(NOT INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Ptah CMake: Installation failed for ${NAME}:\n${INSTALL_ERROR}")
    endif()
    
    # Generate CMake config if one wasn't provided
    ptah_cmake_ensure_config(${NAME} ${INSTALL_DIR})
    
    # Mark as installed
    ptah_mark_library_installed(${NAME})
    
    message(STATUS "Ptah CMake: Successfully built and installed ${NAME}")
endfunction()

function(ptah_cmake_ensure_config NAME INSTALL_DIR)
    # Check multiple possible locations for CMake config files
    set(CONFIG_LOCATIONS 
        "${INSTALL_DIR}/share/cmake/${NAME}"
        "${INSTALL_DIR}/lib/cmake/${NAME}"
        "${INSTALL_DIR}/lib64/cmake/${NAME}"
    )
    
    foreach(CONFIG_DIR ${CONFIG_LOCATIONS})
        set(CONFIG_FILE "${CONFIG_DIR}/${NAME}Config.cmake")
        if(EXISTS ${CONFIG_FILE})
            message(STATUS "Ptah CMake: Using existing CMake config for ${NAME}")
            return()
        endif()
    endforeach()
    
    # If no existing config was found, use the default location for generation
    set(CONFIG_DIR "${INSTALL_DIR}/share/cmake/${NAME}")
    set(CONFIG_FILE "${CONFIG_DIR}/${NAME}Config.cmake")
    
    # Look for pkg-config files as an alternative
    set(PKGCONFIG_FILE "${INSTALL_DIR}/lib/pkgconfig/${NAME}.pc")
    if(EXISTS ${PKGCONFIG_FILE})
        message(STATUS "Ptah CMake: Found pkg-config file, generating CMake config for ${NAME}")
        ptah_cmake_generate_config_from_pkgconfig(${NAME} ${INSTALL_DIR} ${PKGCONFIG_FILE})
        return()
    endif()
    
    # Generate a basic config file
    message(STATUS "Ptah CMake: Generating basic CMake config for ${NAME}")
    ptah_cmake_generate_basic_config(${NAME} ${INSTALL_DIR})
endfunction()

function(ptah_cmake_generate_basic_config NAME INSTALL_DIR)
    set(CONFIG_DIR "${INSTALL_DIR}/share/cmake/${NAME}")
    set(CONFIG_FILE "${CONFIG_DIR}/${NAME}Config.cmake")
    
    file(MAKE_DIRECTORY ${CONFIG_DIR})
    
    # Find libraries (check both lib and lib64 directories)
    file(GLOB_RECURSE STATIC_LIBS "${INSTALL_DIR}/lib/*.a" "${INSTALL_DIR}/lib64/*.a")
    file(GLOB_RECURSE SHARED_LIBS "${INSTALL_DIR}/lib/*.so" "${INSTALL_DIR}/lib64/*.so" "${INSTALL_DIR}/lib/*.dylib" "${INSTALL_DIR}/lib64/*.dylib" "${INSTALL_DIR}/lib/*.dll" "${INSTALL_DIR}/lib64/*.dll")
    
    # Prefer static libraries
    set(LIBS ${STATIC_LIBS})
    if(NOT LIBS)
        set(LIBS ${SHARED_LIBS})
    endif()
    
    # Generate config content
    set(CONFIG_CONTENT "# Generated by Ptah Library Manager
# CMake configuration for ${NAME}

include(CMakeFindDependencyMacro)

# Define the target
if(NOT TARGET ${NAME}::${NAME})
    add_library(${NAME}::${NAME} INTERFACE IMPORTED)
    
    # Set include directories
    set_target_properties(${NAME}::${NAME} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES \"${INSTALL_DIR}/include\"
    )
")
    
    # Add libraries if found
    if(LIBS)
        string(APPEND CONFIG_CONTENT "    
    # Set library paths
    set_target_properties(${NAME}::${NAME} PROPERTIES
        INTERFACE_LINK_LIBRARIES \"")
        
        foreach(LIB ${LIBS})
            string(APPEND CONFIG_CONTENT "${LIB};")
        endforeach()
        
        string(APPEND CONFIG_CONTENT "\"
    )
")
    endif()
    
    string(APPEND CONFIG_CONTENT "endif()

# Mark as found
set(${NAME}_FOUND TRUE)
")
    
    file(WRITE ${CONFIG_FILE} ${CONFIG_CONTENT})
    
    # Generate version file if possible
    set(VERSION_FILE "${CONFIG_DIR}/${NAME}ConfigVersion.cmake")
    set(VERSION_CONTENT "# Generated by Ptah Library Manager
set(PACKAGE_VERSION \"unknown\")
set(PACKAGE_VERSION_COMPATIBLE TRUE)
set(PACKAGE_VERSION_EXACT FALSE)
")
    file(WRITE ${VERSION_FILE} ${VERSION_CONTENT})
endfunction()

function(ptah_cmake_generate_config_from_pkgconfig NAME INSTALL_DIR PKGCONFIG_FILE)
    # This would parse the .pc file and generate a CMake config
    # For now, fall back to basic config
    ptah_cmake_generate_basic_config(${NAME} ${INSTALL_DIR})
endfunction()

# Utility function to detect CMake-specific options for common libraries
function(ptah_cmake_get_library_options NAME OUTPUT_VAR)
    set(OPTIONS "")
    
    # Library-specific options
    if(NAME STREQUAL "SDL3")
        list(APPEND OPTIONS
            -DSDL3_SHARED_ENABLED=OFF
            -DSDL3_STATIC_ENABLED=ON
            -DSDL3_TEST_ENABLED=OFF
            -DSDL3_DISABLE_INSTALL=OFF
            -DSDL3_DISABLE_INSTALL_DOCS=ON
        )
    elseif(NAME STREQUAL "SDL" OR NAME STREQUAL "SDL2")
        list(APPEND OPTIONS
            -DSDL_SHARED=OFF
            -DSDL_STATIC=ON
            -DSDL_TEST=OFF
        )
    elseif(NAME STREQUAL "GLFW" OR NAME STREQUAL "glfw")
        list(APPEND OPTIONS
            -DGLFW_BUILD_EXAMPLES=OFF
            -DGLFW_BUILD_TESTS=OFF
            -DGLFW_BUILD_DOCS=OFF
        )
    elseif(NAME STREQUAL "fmt")
        list(APPEND OPTIONS
            -DFMT_DOC=OFF
            -DFMT_TEST=OFF
        )
    elseif(NAME STREQUAL "spdlog")
        list(APPEND OPTIONS
            -DSPDLOG_BUILD_EXAMPLE=OFF
            -DSPDLOG_BUILD_TESTS=OFF
        )
    endif()
    
    set(${OUTPUT_VAR} ${OPTIONS} PARENT_SCOPE)
endfunction()