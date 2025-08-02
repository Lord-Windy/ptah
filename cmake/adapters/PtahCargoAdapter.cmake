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

# Cargo Build System Adapter for Ptah Library Manager
# Handles building Rust/Cargo-based external libraries (e.g., wgpu-native)

function(ptah_build_cargo_library NAME SOURCE_DIR INSTALL_DIR)
    # Find Cargo
    find_program(CARGO_EXECUTABLE cargo)
    if(NOT CARGO_EXECUTABLE)
        message(FATAL_ERROR "Ptah Cargo: cargo not found. Please install Rust and Cargo.")
    endif()
    
    message(STATUS "Ptah Cargo: Building ${NAME} with cargo")
    
    # Build in release mode for better performance
    execute_process(
        COMMAND ${CARGO_EXECUTABLE} build --release
        WORKING_DIRECTORY ${SOURCE_DIR}
        RESULT_VARIABLE BUILD_RESULT
        OUTPUT_VARIABLE BUILD_OUTPUT
        ERROR_VARIABLE BUILD_ERROR
    )
    
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Ptah Cargo: Build failed for ${NAME}:\n${BUILD_ERROR}")
    endif()
    
    message(STATUS "Ptah Cargo: Installing ${NAME}")
    
    # Manually install since Cargo doesn't have a standard install command
    ptah_cargo_install_artifacts(${NAME} ${SOURCE_DIR} ${INSTALL_DIR})
    
    # Generate CMake config
    ptah_cargo_generate_cmake_config(${NAME} ${INSTALL_DIR})
    
    # Mark as installed
    ptah_mark_library_installed(${NAME})
    
    message(STATUS "Ptah Cargo: Successfully built and installed ${NAME}")
endfunction()

function(ptah_cargo_install_artifacts NAME SOURCE_DIR INSTALL_DIR)
    set(TARGET_DIR "${SOURCE_DIR}/target/release")
    
    # Install libraries
    file(MAKE_DIRECTORY "${INSTALL_DIR}/lib")
    
    # Find built libraries (static and dynamic)
    file(GLOB_RECURSE STATIC_LIBS 
        "${TARGET_DIR}/*.a"
        "${TARGET_DIR}/*.lib"
    )
    file(GLOB_RECURSE DYNAMIC_LIBS 
        "${TARGET_DIR}/*.so"
        "${TARGET_DIR}/*.dylib"
        "${TARGET_DIR}/*.dll"
    )
    
    # Install libraries
    foreach(LIB ${STATIC_LIBS} ${DYNAMIC_LIBS})
        get_filename_component(LIB_NAME ${LIB} NAME)
        file(COPY ${LIB} DESTINATION "${INSTALL_DIR}/lib")
        message(STATUS "Ptah Cargo: Installed library ${LIB_NAME}")
    endforeach()
    
    # Install headers if they exist
    set(HEADER_LOCATIONS
        "${SOURCE_DIR}/include"
        "${SOURCE_DIR}/headers"
        "${SOURCE_DIR}/ffi"
        "${SOURCE_DIR}/bindings"
    )
    
    foreach(HEADER_DIR ${HEADER_LOCATIONS})
        if(EXISTS ${HEADER_DIR})
            file(MAKE_DIRECTORY "${INSTALL_DIR}/include")
            file(GLOB_RECURSE HEADERS "${HEADER_DIR}/*.h" "${HEADER_DIR}/*.hpp")
            if(HEADERS)
                # Copy maintaining directory structure
                foreach(HEADER ${HEADERS})
                    file(RELATIVE_PATH REL_PATH ${HEADER_DIR} ${HEADER})
                    get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
                    file(MAKE_DIRECTORY "${INSTALL_DIR}/include/${REL_DIR}")
                    file(COPY ${HEADER} DESTINATION "${INSTALL_DIR}/include/${REL_DIR}")
                endforeach()
                message(STATUS "Ptah Cargo: Installed headers from ${HEADER_DIR}")
            endif()
        endif()
    endforeach()
    
    # Install executables if this is a binary crate
    file(GLOB EXECUTABLES "${TARGET_DIR}/*")
    foreach(EXEC ${EXECUTABLES})
        if(IS_EXECUTABLE ${EXEC} AND NOT IS_DIRECTORY ${EXEC})
            get_filename_component(EXEC_NAME ${EXEC} NAME)
            # Skip files with extensions (likely libraries)
            get_filename_component(EXEC_EXT ${EXEC} EXT)
            if(EXEC_EXT STREQUAL "")
                file(MAKE_DIRECTORY "${INSTALL_DIR}/bin")
                file(COPY ${EXEC} DESTINATION "${INSTALL_DIR}/bin")
                message(STATUS "Ptah Cargo: Installed executable ${EXEC_NAME}")
            endif()
        endif()
    endforeach()
endfunction()

function(ptah_cargo_generate_cmake_config NAME INSTALL_DIR)
    set(CONFIG_DIR "${INSTALL_DIR}/share/cmake/${NAME}")
    set(CONFIG_FILE "${CONFIG_DIR}/${NAME}Config.cmake")
    
    file(MAKE_DIRECTORY ${CONFIG_DIR})
    
    # Find installed libraries
    file(GLOB_RECURSE STATIC_LIBS "${INSTALL_DIR}/lib/*.a" "${INSTALL_DIR}/lib/*.lib")
    file(GLOB_RECURSE SHARED_LIBS "${INSTALL_DIR}/lib/*.so" "${INSTALL_DIR}/lib/*.dylib" "${INSTALL_DIR}/lib/*.dll")
    
    # Prefer static libraries for Rust crates
    set(LIBS ${STATIC_LIBS})
    if(NOT LIBS)
        set(LIBS ${SHARED_LIBS})
    endif()
    
    # Determine library type
    set(LIB_TYPE "STATIC")
    if(SHARED_LIBS AND NOT STATIC_LIBS)
        set(LIB_TYPE "SHARED")
    endif()
    
    # Generate config content
    set(CONFIG_CONTENT "# Generated by Ptah Library Manager
# CMake configuration for ${NAME} (Cargo/Rust library)

include(CMakeFindDependencyMacro)

# Find required system libraries for Rust crates
if(WIN32)
    find_library(WS2_32_LIBRARY ws2_32)
    find_library(USERENV_LIBRARY userenv)
    find_library(ADVAPI32_LIBRARY advapi32)
    set(SYSTEM_LIBS \${WS2_32_LIBRARY} \${USERENV_LIBRARY} \${ADVAPI32_LIBRARY})
elseif(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(SECURITY Security)
    set(SYSTEM_LIBS \${CORE_FOUNDATION} \${SECURITY})
else()
    # Linux
    find_library(DL_LIBRARY dl)
    find_library(PTHREAD_LIBRARY pthread)
    find_library(M_LIBRARY m)
    set(SYSTEM_LIBS \${DL_LIBRARY} \${PTHREAD_LIBRARY} \${M_LIBRARY})
endif()

# Define the target
if(NOT TARGET ${NAME}::${NAME})
")

    if(LIBS)
        # Get the first library to determine the main library name
        list(GET LIBS 0 MAIN_LIB)
        
        string(APPEND CONFIG_CONTENT "    add_library(${NAME}::${NAME} ${LIB_TYPE} IMPORTED)
    
    # Set library location
    set_target_properties(${NAME}::${NAME} PROPERTIES
        IMPORTED_LOCATION \"${MAIN_LIB}\"
")
        
        # Add include directories if they exist
        if(EXISTS "${INSTALL_DIR}/include")
            string(APPEND CONFIG_CONTENT "        INTERFACE_INCLUDE_DIRECTORIES \"${INSTALL_DIR}/include\"
")
        endif()
        
        # Add system libraries for Rust crates
        string(APPEND CONFIG_CONTENT "        INTERFACE_LINK_LIBRARIES \"\${SYSTEM_LIBS}\"
")
        
        string(APPEND CONFIG_CONTENT "    )
")
    else()
        # Header-only or executable-only
        string(APPEND CONFIG_CONTENT "    add_library(${NAME}::${NAME} INTERFACE IMPORTED)
    
    # Set include directories
    set_target_properties(${NAME}::${NAME} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES \"${INSTALL_DIR}/include\"
        INTERFACE_LINK_LIBRARIES \"\${SYSTEM_LIBS}\"
    )
")
    endif()
    
    string(APPEND CONFIG_CONTENT "endif()

# Mark as found
set(${NAME}_FOUND TRUE)
")
    
    file(WRITE ${CONFIG_FILE} ${CONFIG_CONTENT})
    
    # Generate version file
    set(VERSION_FILE "${CONFIG_DIR}/${NAME}ConfigVersion.cmake")
    set(VERSION_CONTENT "# Generated by Ptah Library Manager
# Version information for ${NAME} (Cargo/Rust library)

set(PACKAGE_VERSION \"unknown\")
set(PACKAGE_VERSION_COMPATIBLE TRUE)
set(PACKAGE_VERSION_EXACT FALSE)
")
    file(WRITE ${VERSION_FILE} ${VERSION_CONTENT})
endfunction()

# Utility function to detect Cargo-specific features
function(ptah_cargo_get_library_features NAME SOURCE_DIR OUTPUT_VAR)
    set(FEATURES "")
    
    # Read Cargo.toml to detect features
    set(CARGO_TOML "${SOURCE_DIR}/Cargo.toml")
    if(EXISTS ${CARGO_TOML})
        file(READ ${CARGO_TOML} CARGO_CONTENT)
        
        # Look for common features that might be useful for C interop
        if(CARGO_CONTENT MATCHES "\\[features\\]")
            # Common C interop features
            if(CARGO_CONTENT MATCHES "\"ffi\"")
                list(APPEND FEATURES "--features" "ffi")
            endif()
            if(CARGO_CONTENT MATCHES "\"c-api\"")
                list(APPEND FEATURES "--features" "c-api")
            endif()
            if(CARGO_CONTENT MATCHES "\"capi\"")
                list(APPEND FEATURES "--features" "capi")
            endif()
        endif()
    endif()
    
    set(${OUTPUT_VAR} ${FEATURES} PARENT_SCOPE)
endfunction()