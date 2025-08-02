# Ptah Examples

This directory contains example applications demonstrating various aspects of the Ptah framework, including library usage and external dependency integration.

## Available Examples

### Core Library Examples

- **[typesafe_example](typesafe_example/)** - Demonstrates type-safe memory management with Samrena
- **[iterator_example](iterator_example/)** - Shows iterator patterns with Ptah data structures  
- **[pearl_example](pearl_example/)** - Pearl set data structure usage

### External Library Integration Examples

- **[sdl_example](sdl_example/)** - SDL3 integration using Ptah Library Manager

## External Library Integration

The SDL example demonstrates the complete workflow for integrating external libraries using the Ptah Library Manager system (`@cmake/`). This pattern can be applied to any external library.

### Key Concepts Demonstrated

1. **Ptah Library Manager Usage**
   ```cmake
   include(${CMAKE_SOURCE_DIR}/cmake/PtahLibraryManager.cmake)
   
   ptah_add_library(
       NAME SDL3
       GIT_REPOSITORY https://github.com/libsdl-org/SDL
       VERSION release-3.2.18
       BUILD_SYSTEM cmake
       CMAKE_ARGS
           -DSDL_STATIC=ON
           -DSDL_SHARED=OFF
           -DSDL_TEST=OFF
           -DSDL_EXAMPLES=OFF
   )
   
   ptah_find_library(SDL3)
   ```

2. **Integration with Ptah Build System**
   ```cmake
   ptah_add_executable(sdl_example
       SOURCES
           sdl_example.c
       DEPENDENCIES
           SDL3::SDL3
   )
   ```

3. **Automatic Dependency Management**
   - Libraries are automatically downloaded and built
   - Version pinning ensures reproducible builds
   - External dependencies are isolated in `external_dependencies/`

## Building Examples

### Build All Examples

```bash
mkdir build
cd build
cmake ..
make
```

### Build Specific Example

```bash
mkdir build
cd build
cmake ..
make sdl_example  # or any other example name
```

### Run Examples

```bash
# From project root after building
./bin/sdl_example
./bin/typesafe_example
./bin/iterator_example
./bin/pearl_example
```

## Creating New Examples

### Using Internal Libraries Only

```cmake
# apps/examples/my_example/CMakeLists.txt
ptah_add_executable(my_example
    SOURCES
        my_example.c
    DEPENDENCIES
        samrena     # Memory arena management
        datazoo     # Data structures (honeycomb, pearl, etc.)
)
```

### Using External Libraries

```cmake
# apps/examples/my_external_example/CMakeLists.txt
include(${CMAKE_SOURCE_DIR}/cmake/PtahLibraryManager.cmake)

# Add external library
ptah_add_library(
    NAME my_external_lib
    GIT_REPOSITORY https://github.com/org/library
    VERSION v1.0.0
    BUILD_SYSTEM cmake
)

ptah_find_library(my_external_lib)

# Create executable
ptah_add_executable(my_external_example
    SOURCES
        my_external_example.c
    DEPENDENCIES
        samrena                    # Ptah library
        my_external_lib::library   # External library
)
```

### Register New Example

Add your new example to the parent CMakeLists.txt:

```cmake
# apps/examples/CMakeLists.txt
add_subdirectory(typesafe_example)
add_subdirectory(iterator_example)
add_subdirectory(pearl_example)
add_subdirectory(sdl_example)
add_subdirectory(my_new_example)  # Add this line
```

## Documentation

- **[External Libraries Guide](../../docs/EXTERNAL_LIBRARIES.md)** - Comprehensive guide to external library integration
- **[Main Project README](../../CLAUDE.md)** - Project overview and build instructions
- **Individual Example READMEs** - Specific documentation for each example

## Common External Libraries

Here are some popular libraries you might want to integrate:

### Graphics and Multimedia
- **SDL3**: Cross-platform multimedia (demonstrated in sdl_example)
- **GLFW**: OpenGL window management
- **OpenGL/Vulkan**: Graphics APIs
- **Dear ImGui**: Immediate mode GUI

### Utility Libraries  
- **fmt**: Modern C++ formatting
- **nlohmann/json**: JSON parsing
- **cURL**: HTTP client
- **sqlite3**: Embedded database

### Math and Science
- **Eigen**: Linear algebra
- **FFTW**: Fast Fourier Transform
- **GSL**: GNU Scientific Library

Each can be integrated following the same pattern demonstrated in the SDL example.

## Best Practices

1. **Version Pinning**: Always specify exact versions for reproducible builds
2. **Minimal Configuration**: Only build what you need from external libraries
3. **Static Linking**: Prefer static libraries for simpler deployment
4. **Documentation**: Document external library choices and configuration
5. **Testing**: Verify examples work across different platforms

## Troubleshooting

- **Build Issues**: Check `external_dependencies/build/[library]/` for logs
- **Missing Dependencies**: Ensure system has required development packages  
- **Version Conflicts**: Use specific CMAKE_ARGS to resolve conflicts
- **Clean Rebuild**: Remove `external_dependencies/` and rebuild when in doubt

For detailed troubleshooting, see the [External Libraries Guide](../../docs/EXTERNAL_LIBRARIES.md).