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

# Platform detection for virtual memory support
if(WIN32)
    set(SAMRENA_HAS_VIRTUAL_MEMORY_DEFAULT ON)
    set(SAMRENA_VM_PLATFORM "win32")
elseif(UNIX)
    set(SAMRENA_HAS_VIRTUAL_MEMORY_DEFAULT ON)
    if(APPLE)
        set(SAMRENA_VM_PLATFORM "darwin")
    else()
        set(SAMRENA_VM_PLATFORM "posix")
    endif()
else()
    set(SAMRENA_HAS_VIRTUAL_MEMORY_DEFAULT OFF)
    set(SAMRENA_VM_PLATFORM "none")
endif()

# User option to override
option(SAMRENA_ENABLE_VIRTUAL 
       "Enable virtual memory adapter" 
       ${SAMRENA_HAS_VIRTUAL_MEMORY_DEFAULT})

# Feature summary
if(SAMRENA_ENABLE_VIRTUAL)
    message(STATUS "Samrena: Virtual memory adapter enabled (${SAMRENA_VM_PLATFORM})")
else()
    message(STATUS "Samrena: Virtual memory adapter disabled")
endif()

# Conditional source files
set(SAMRENA_SOURCES
    src/samrena.c
    src/samrena_vector.c
    src/adapters/chained_adapter.c
)

if(SAMRENA_ENABLE_VIRTUAL)
    list(APPEND SAMRENA_SOURCES
        src/adapters/virtual_adapter.c
    )
endif()

ptah_add_library(samrena
    SOURCES ${SAMRENA_SOURCES}
    PUBLIC_HEADERS
        include/samrena.h
)

# Generate config header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/samrena_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/samrena_config.h"
    @ONLY
)

# Add generated header to include path
target_include_directories(samrena PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Build tests if testing is enabled
if(BUILD_TESTING)
    enable_testing()
    
    add_executable(samrena_test
        test/samrena_test.c
    )
    target_link_libraries(samrena_test PRIVATE samrena)
    add_test(NAME samrena_test COMMAND samrena_test)
    
    add_executable(samrena_vector_test
        test/samrena_vector_test.c
    )
    target_link_libraries(samrena_vector_test PRIVATE samrena)
    add_test(NAME samrena_vector_test COMMAND samrena_vector_test)
    
    add_executable(samrena_feature_test
        test/test_features.c
    )
    target_link_libraries(samrena_feature_test PRIVATE samrena)
    add_test(NAME samrena_feature_test COMMAND samrena_feature_test)
endif()