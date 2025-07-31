# Task: CMake Platform Detection

## Overview
Implement CMake configuration to detect platform capabilities and enable/disable the virtual memory adapter accordingly.

## Requirements
- Detect operating system support
- Allow manual override
- Generate config header
- Clear feature reporting

## Implementation Details

### 1. Update CMakeLists.txt
```cmake
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
```

### 2. Generate Configuration Header
```cmake
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
```

### 3. Create Config Header Template
```c
// src/samrena_config.h.in
#ifndef SAMRENA_CONFIG_H
#define SAMRENA_CONFIG_H

// Virtual memory support
#cmakedefine SAMRENA_ENABLE_VIRTUAL
#define SAMRENA_VM_PLATFORM "@SAMRENA_VM_PLATFORM@"

// Platform-specific includes
#ifdef SAMRENA_ENABLE_VIRTUAL
    #if defined(_WIN32)
        #define SAMRENA_VM_WIN32
    #elif defined(__APPLE__)
        #define SAMRENA_VM_DARWIN
    #elif defined(__linux__) || defined(__unix__)
        #define SAMRENA_VM_POSIX
    #endif
#endif

// Default page size
#ifndef SAMRENA_DEFAULT_PAGE_SIZE
    #define SAMRENA_DEFAULT_PAGE_SIZE (64 * 1024)  // 64KB
#endif

#endif // SAMRENA_CONFIG_H
```

### 4. Update Source List
```cmake
# Conditional source files
set(SAMRENA_SOURCES
    src/samrena.c
    src/adapter_chained.c
)

if(SAMRENA_ENABLE_VIRTUAL)
    list(APPEND SAMRENA_SOURCES
        src/adapter_virtual.c
    )
    
    if(WIN32)
        list(APPEND SAMRENA_SOURCES src/adapter_virtual_win32.c)
    else()
        list(APPEND SAMRENA_SOURCES src/adapter_virtual_posix.c)
    endif()
endif()

ptah_add_library(samrena
    SOURCES ${SAMRENA_SOURCES}
    PUBLIC_HEADERS include/samrena.h
)
```

## Location
- `libs/samrena/CMakeLists.txt` - Build configuration
- `libs/samrena/src/samrena_config.h.in` - Config template

## Dependencies
- None (build system task)

## Verification
- [ ] Correctly detects Windows
- [ ] Correctly detects Linux
- [ ] Correctly detects macOS
- [ ] Manual override works
- [ ] Config header generated properly

## Notes
- Consider detecting specific features (mmap, VirtualAlloc)
- May need to handle BSD variants
- Document in README how to disable virtual memory