# CMake Library Management System

## Overview

This document describes the modular CMake build system architecture used in the
Ptah project. The system is designed to provide Bazel-like modularity with
automatic discovery of libraries, applications, and tests, while maintaining
CMake's flexibility and standard tooling support.

### Design Philosophy

1. **Automatic Discovery**: Components are discovered automatically based on
   directory structure
2. **Consistent Interfaces**: Helper functions provide uniform configuration
   for all targets
3. **Selective Installation**: Components opt-in to installation via explicit
   flags
4. **Package Support**: Generates CMake package files for consumption by other
   projects
5. **Modularity**: Each component is self-contained with its own CMakeLists.txt

## Core Components

### 1. Helper Function: `ptah_add_library()`

Creates libraries with consistent settings, include directories, and optional
installation rules.

#### Signature
```cmake
ptah_add_library(name
    [STATIC|SHARED|INTERFACE]
    [INSTALL]
    SOURCES source1.c source2.c ...
    PUBLIC_HEADERS header1.h header2.h ...
    DEPENDENCIES dep1 dep2 ...
)
```

#### Parameters

- **name**: The library target name
- **STATIC**: Create a static library (`.a` on Unix)
- **SHARED**: Create a shared library (`.so`/`.dylib`/`.dll`)
- **INTERFACE**: Create a header-only library
- **INSTALL**: Enable installation rules for this library
- **SOURCES**: List of source files (`.c`, `.cpp`, etc.)
- **PUBLIC_HEADERS**: Headers to install for consumers
- **DEPENDENCIES**: Libraries this target depends on (linked with `PUBLIC`)

#### Implementation Details

```cmake
function(ptah_add_library name)
    set(options STATIC SHARED INTERFACE INSTALL)
    set(oneValueArgs)
    set(multiValueArgs SOURCES PUBLIC_HEADERS DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Create the library target
    if(ARG_INTERFACE)
        add_library(${name} INTERFACE)
    elseif(ARG_STATIC)
        add_library(${name} STATIC ${ARG_SOURCES})
    elseif(ARG_SHARED)
        add_library(${name} SHARED ${ARG_SOURCES})
    else()
        add_library(${name} ${ARG_SOURCES})  # Respects BUILD_SHARED_LIBS
    endif()

    # Set include directories (if not INTERFACE)
    if(NOT ARG_INTERFACE)
        target_include_directories(${name}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )
    endif()

    # Link dependencies
    if(ARG_DEPENDENCIES)
        target_link_libraries(${name} PUBLIC ${ARG_DEPENDENCIES})
    endif()

    # Install rules (opt-in only)
    if(ARG_INSTALL)
        if(ARG_PUBLIC_HEADERS)
            set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${ARG_PUBLIC_HEADERS}")
        endif()

        install(TARGETS ${name}
            EXPORT ptahTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
endfunction()
```

#### Key Features

**Generator Expressions for Include Paths**:

- `BUILD_INTERFACE`: Used during build (points to source tree)
- `INSTALL_INTERFACE`: Used when installed (points to install tree)

**Opt-in Installation**: Libraries must explicitly specify `INSTALL` to be
included in the install target. This allows for:
- Internal/private libraries that shouldn't be exposed
- Test utilities that don't need installation
- Clean separation between public API and implementation

**Public Dependencies**: Dependencies are linked with `PUBLIC` visibility,
meaning:
- Consumers automatically get transitive dependencies
- Include paths propagate correctly
- Works naturally with CMake's modern target-based approach

### 2. Helper Function: `ptah_add_executable()`

Creates executables with consistent dependency management and optional
installation.

#### Signature
```cmake
ptah_add_executable(name
    [INSTALL]
    SOURCES source1.c source2.c ...
    DEPENDENCIES dep1 dep2 ...
)
```

#### Parameters

- **name**: The executable target name
- **INSTALL**: Enable installation rules for this executable
- **SOURCES**: List of source files
- **DEPENDENCIES**: Libraries to link against (linked with `PRIVATE`)

#### Implementation Details

```cmake
function(ptah_add_executable name)
    set(options INSTALL)
    set(oneValueArgs)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${name} ${ARG_SOURCES})

    if(ARG_DEPENDENCIES)
        target_link_libraries(${name} PRIVATE ${ARG_DEPENDENCIES})
    endif()

    # Only install if INSTALL option is specified
    if(ARG_INSTALL)
        install(TARGETS ${name}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
endfunction()
```

#### Key Features

**Private Dependencies**: Executables link dependencies with `PRIVATE` because:
- They are final consumers (not re-exported)
- Reduces unnecessary exposure of internal dependencies

**Selective Installation**: Examples and test executables typically omit
`INSTALL`, while production tools include it.

### 3. Automatic Discovery: `add_subdirectories_recursively()`

Recursively discovers and adds subdirectories containing `CMakeLists.txt`
files.

#### Implementation

```cmake
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
```

#### Usage

```cmake
# Automatically discover and add all subdirectories with CMakeLists.txt
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
    add_subdirectories_recursively(libs)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/apps)
    add_subdirectories_recursively(apps)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    add_subdirectories_recursively(tools)
endif()
```

#### How It Works

1. Scans a top-level directory (e.g., `libs/`)
2. For each subdirectory:
   - If it contains `CMakeLists.txt`, add it as a subproject
   - If it doesn't, recursively scan deeper
3. Allows arbitrary nesting (e.g., `libs/category/library/`)

#### Benefits

- **Zero Configuration**: Just create a `CMakeLists.txt` and it's discovered
- **Flexible Organization**: Organize by feature, domain, or any hierarchy
- **Bazel-like Feel**: Each component is independent and self-describing
- **Scalability**: Works for small and large projects

## Directory Structure

### Standard Layout

```
project_root/
├── CMakeLists.txt              # Root build configuration
├── cmake/
│   └── projectConfig.cmake.in  # Package config template
├── libs/                       # Libraries
│   ├── core/
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   │   └── core.h
│   │   └── src/
│   │       └── core.c
│   └── utils/
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── utils.h
│       └── src/
│           └── utils.c
├── apps/                       # Applications
│   └── myapp/
│       ├── CMakeLists.txt
│       └── src/
│           └── main.c
├── tools/                      # Development tools
│   └── codegen/
│       ├── CMakeLists.txt
│       └── src/
│           └── main.c
└── tests/                      # Test suites
    ├── core_tests/
    │   ├── CMakeLists.txt
    │   └── test_core.c
    └── utils_tests/
        ├── CMakeLists.txt
        └── test_utils.c
```

### Component-Level Structure

Each library follows this pattern:

```
library_name/
├── CMakeLists.txt      # Build configuration
├── include/            # Public headers
│   └── library_name.h
├── src/                # Implementation
│   ├── internal.h      # Private headers
│   └── impl.c
└── test/               # Optional tests (can be in tests/ instead)
    └── test_library.c
```

## Package Configuration

The system generates CMake package files for `find_package()` support, allowing
other projects to consume your libraries easily.

### Export Configuration

```cmake
# Install export targets
install(EXPORT ptahTargets
    FILE ptahTargets.cmake
    NAMESPACE ptah::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ptah
)

# Generate package version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ptahConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Generate package config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ptahConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ptahConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ptah
)

# Install package config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ptahConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ptahConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ptah
)
```

### Package Config Template

Create `cmake/ptahConfig.cmake.in`:

```cmake
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/ptahTargets.cmake")

check_required_components(ptah)
```

### Using the Package

After installation, other projects can use:

```cmake
find_package(ptah 0.1.0 REQUIRED)

add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE ptah::core ptah::utils)
```

## Practical Examples

### Example 1: Simple Library

`libs/mylib/CMakeLists.txt`:

```cmake
ptah_add_library(mylib
    INSTALL
    SOURCES
        src/mylib.c
        src/helpers.c
    PUBLIC_HEADERS
        include/mylib.h
    DEPENDENCIES
        # No dependencies
)
```

### Example 2: Library with Dependencies

`libs/advanced/CMakeLists.txt`:

```cmake
ptah_add_library(advanced
    INSTALL
    SOURCES
        src/advanced.c
    PUBLIC_HEADERS
        include/advanced.h
    DEPENDENCIES
        mylib           # Internal library
        PostgreSQL::PostgreSQL  # External library
)
```

### Example 3: Header-Only Library

`libs/templates/CMakeLists.txt`:

```cmake
ptah_add_library(templates
    INTERFACE
    INSTALL
    PUBLIC_HEADERS
        include/templates.h
)

# For interface libraries, manually set include directories
target_include_directories(templates
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
```

### Example 4: Application

`apps/myapp/CMakeLists.txt`:

```cmake
ptah_add_executable(myapp
    INSTALL
    SOURCES
        src/main.c
        src/config.c
    DEPENDENCIES
        mylib
        advanced
)
```

### Example 5: Test Executable (Not Installed)

`tests/mylib_tests/CMakeLists.txt`:

```cmake
ptah_add_executable(test_mylib
    SOURCES
        test_basic.c
        test_advanced.c
    DEPENDENCIES
        mylib
)

# Register with CTest
add_test(NAME mylib_basic COMMAND test_mylib)
```

### Example 6: Internal/Private Library

`libs/internal_utils/CMakeLists.txt`:

```cmake
# Note: no INSTALL flag - this library is private
ptah_add_library(internal_utils
    SOURCES
        src/utils.c
    PUBLIC_HEADERS
        include/internal_utils.h
)
```

## Adapting for Microservices

### Architecture Overview

For a microservices architecture, you can adapt this system as follows:

```
microservices_project/
├── CMakeLists.txt              # Root orchestrator
├── libs/                       # Shared libraries
│   ├── common/                 # Shared utilities
│   ├── models/                 # Shared data models
│   └── clients/                # Service clients
├── services/                   # Microservices (instead of apps/)
│   ├── auth-service/
│   │   ├── CMakeLists.txt
│   │   ├── src/
│   │   │   └── main.c
│   │   └── Dockerfile          # Container definition
│   ├── user-service/
│   │   ├── CMakeLists.txt
│   │   ├── src/
│   │   │   └── main.c
│   │   └── Dockerfile
│   └── api-gateway/
│       ├── CMakeLists.txt
│       ├── src/
│       │   └── main.c
│       └── Dockerfile
└── tests/
    └── integration/            # Integration tests
```

### Root CMakeLists.txt Adaptation

```cmake
cmake_minimum_required(VERSION 3.16)
project(microservices VERSION 1.0.0 LANGUAGES C)

# ... (include all the helper functions) ...

# Discover shared libraries
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
    add_subdirectories_recursively(libs)
endif()

# Discover services (renamed from apps)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/services)
    add_subdirectories_recursively(services)
endif()

# Tests
if(BUILD_TESTING)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
        add_subdirectories_recursively(tests)
    endif()
endif()
```

### Service Definition Example

`services/auth-service/CMakeLists.txt`:

```cmake
ptah_add_executable(auth-service
    INSTALL
    SOURCES
        src/main.c
        src/auth.c
        src/jwt.c
    DEPENDENCIES
        common          # Shared library
        models          # Shared data models
        PostgreSQL::PostgreSQL
)

# Service-specific configuration
target_compile_definitions(auth-service PRIVATE
    SERVICE_NAME="auth-service"
    SERVICE_PORT=8001
)

# Install configuration files
install(FILES
    config/auth-service.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/microservices
)
```

### Shared Library for Services

`libs/common/CMakeLists.txt`:

```cmake
ptah_add_library(common
    INSTALL
    SOURCES
        src/logging.c
        src/config.c
        src/http_server.c
    PUBLIC_HEADERS
        include/common/logging.h
        include/common/config.h
        include/common/http_server.h
    DEPENDENCIES
        libcurl::libcurl
)
```

### Build and Deploy Workflow

```bash
# Build all services
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Build specific service
make auth-service

# Install to staging directory
make install DESTDIR=/tmp/staging

# Services are in /tmp/staging/usr/local/bin/
# Configuration in /tmp/staging/usr/local/etc/microservices/
```

### Container Integration

Each service can have a `Dockerfile`:

```dockerfile
FROM alpine:latest

RUN apk add --no-cache libpq libcurl

COPY bin/auth-service /usr/local/bin/
COPY etc/microservices/auth-service.conf /etc/

EXPOSE 8001

CMD ["/usr/local/bin/auth-service"]
```

Build script example:

```bash
#!/bin/bash
# build-service.sh

SERVICE=$1
cmake --build build --target ${SERVICE}
mkdir -p staging/bin staging/etc
cp build/bin/${SERVICE} staging/bin/
docker build -t myorg/${SERVICE}:latest -f services/${SERVICE}/Dockerfile staging/
```

## Advanced Patterns

### Pattern 1: Service Groups

Organize related services:

```
services/
├── core/
│   ├── auth-service/
│   └── user-service/
├── business/
│   ├── order-service/
│   └── inventory-service/
└── gateway/
    └── api-gateway/
```

The recursive discovery handles this automatically.

### Pattern 2: Shared Protocols

```
libs/
├── protocols/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── grpc_messages.h
│   │   └── rest_api.h
│   └── src/
│       └── serialization.c
```

All services can depend on `protocols` library.

### Pattern 3: Optional Dependencies

```cmake
# In service CMakeLists.txt
if(TARGET monitoring)
    list(APPEND SERVICE_DEPS monitoring)
endif()

ptah_add_executable(my-service
    SOURCES src/main.c
    DEPENDENCIES
        common
        ${SERVICE_DEPS}
)
```

### Pattern 4: Multi-Language Support

For polyglot microservices:

```cmake
# Root CMakeLists.txt
project(microservices VERSION 1.0.0 LANGUAGES C CXX)

# Set standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
```

Then services can be written in C or C++ as needed.

## Best Practices

### 1. Naming Conventions

- **Libraries**: Use lowercase with underscores (e.g., `user_management`)
- **Services**: Use lowercase with hyphens (e.g., `auth-service`)
- **Headers**: Match library name (e.g., `user_management.h` for
  `user_management`)

### 2. Dependency Management

- Keep dependency trees shallow
- Use `PUBLIC` for libraries, `PRIVATE` for executables
- Document external dependencies in root CMakeLists.txt

### 3. Version Management

```cmake
# In library CMakeLists.txt
set_target_properties(mylib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
```

### 4. Testing Strategy

```
tests/
├── unit/           # Unit tests per library
├── integration/    # Cross-service integration tests
└── e2e/           # End-to-end tests
```

### 5. Configuration Management

```cmake
# Install configuration templates
install(FILES
    config/${SERVICE_NAME}.conf.template
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/microservices
)
```

### 6. Documentation

Each service should include:
- README.md describing the service
- API documentation
- Configuration examples

## Migration Guide

### From Existing CMake Project

1. **Adopt helper functions**: Copy the `ptah_add_library()` and
   `ptah_add_executable()` functions
2. **Restructure directories**: Move code into `libs/`, `apps/`, `tests/`
3. **Update target definitions**: Replace `add_library()` with
   `ptah_add_library()`
4. **Add discovery**: Use `add_subdirectories_recursively()`
5. **Configure exports**: Set up package configuration files

### From Monolithic Build

1. **Identify modules**: Break down into logical libraries
2. **Create library CMakeLists**: One per module
3. **Define dependencies**: Use `DEPENDENCIES` parameter
4. **Enable discovery**: Let CMake find them automatically
5. **Iterative migration**: Convert one module at a time

## Troubleshooting

### Common Issues

**Issue**: Library not found by dependent

**Solution**: Ensure the library's subdirectory is added before the dependent's
subdirectory. CMake processes in order.

**Issue**: Headers not found

**Solution**: Verify `PUBLIC_HEADERS` is set and `target_include_directories()`
uses generator expressions correctly.

**Issue**: Service can't link shared library

**Solution**: Check `BUILD_SHARED_LIBS` option and ensure `RPATH` is configured
for shared libraries.

**Issue**: Package not found after installation

**Solution**: Ensure `CMAKE_PREFIX_PATH` includes the installation directory,
or install to a standard location.

## Conclusion

This CMake library management system provides:

- **Modularity**: Each component is self-contained
- **Scalability**: Automatic discovery supports growth
- **Flexibility**: Opt-in installation and dependencies
- **Standard Compliance**: Uses CMake best practices
- **Package Support**: Easy consumption by other projects

For microservices, it offers:

- **Service Independence**: Each service is a separate executable
- **Shared Code**: Common libraries eliminate duplication
- **Build Efficiency**: Build only what you need
- **Deployment Flexibility**: Install services independently

This system scales from small projects to large microservice architectures
while maintaining clarity and consistency.

## References

- CMake Documentation: https://cmake.org/documentation/
- Modern CMake Guide: https://cliutils.gitlab.io/modern-cmake/
- CMake Package Configuration:
  https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html
