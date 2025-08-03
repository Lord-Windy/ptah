# Requirements: Ptah GUI Renderer Library

## Overview

An immediate-mode rendering library for the Ptah ecosystem that provides a foundation for building GUI applications using Clay UI for layout, SDL3 for windowing, and WebGPU (via webgpu-native) for graphics rendering, with full integration into Ptah's arena-based memory management.

## Functional Requirements

### Core Functionality
- **FR1**: Initialize and manage immediate-mode rendering pipeline
  - Input: Configuration parameters (window size, title, GPU preferences), Samrena arena for allocations
  - Output: Initialized renderer context ready for immediate-mode drawing
  - Constraints: Must support fallback if preferred GPU unavailable, all allocations via provided arena

- **FR2**: Integrate Clay UI layout system (immediate-mode)
  - Input: Clay UI element definitions and layout constraints per frame
  - Output: Computed layout dimensions and positions for current frame
  - Constraints: Layout recalculated every frame, no caching

- **FR3**: Render Clay UI elements via WebGPU
  - Input: Clay UI render commands for current frame
  - Output: Rendered frame to SDL3 window
  - Constraints: Must maintain 60 FPS for typical GUI applications (up to 1000 elements)

- **FR4**: Handle window events and input
  - Input: SDL3 events (mouse, keyboard, window events)
  - Output: Translated events for Clay UI interaction
  - Constraints: Event handling must not block rendering

- **FR5**: Built-in shader system
  - Input: UI element types (rectangles, text, images, rounded rectangles)
  - Output: Appropriate shader selection and uniform binding
  - Constraints: Fixed set of built-in shaders, extensibility deferred to future version

- **FR6**: Text rendering via CPU rasterization
  - Input: Font data, text strings, size, style attributes
  - Output: Rasterized glyphs uploaded as textures to GPU
  - Constraints: Use stb_truetype or similar, cache glyphs within frame arena

- **FR7**: Simple texture resource management
  - Input: Image data or file paths
  - Output: GPU texture handles
  - Constraints: Manual load/unload, no automatic reference counting

### User Interface
- **UI1**: Renderer API (Ptah-style)
  - Type: C API using Ptah conventions
  - Key Functions:
    - `ptah_renderer_create(SamrenaArena* arena, PtahRendererConfig* config)` - Initialize renderer
    - `ptah_renderer_begin_frame(PtahRenderer* renderer)` - Start new frame
    - `ptah_renderer_end_frame(PtahRenderer* renderer)` - Submit frame for presentation
    - `ptah_renderer_process_events(PtahRenderer* renderer)` - Handle SDL events
    - `ptah_renderer_draw_commands(PtahRenderer* renderer, ClayRenderCommandArray* commands)` - Draw Clay UI commands
    - `ptah_renderer_load_texture(PtahRenderer* renderer, const char* path)` - Load texture
    - `ptah_renderer_unload_texture(PtahRenderer* renderer, PtahTextureHandle handle)` - Unload texture
    - `ptah_renderer_destroy(PtahRenderer* renderer)` - Clean up resources

## Non-Functional Requirements

### Performance
- **NFR1**: Maintain 60 FPS for applications with up to 1000 UI elements
- **NFR2**: Frame preparation time < 8ms on modern GPUs
- **NFR3**: Per-frame arena memory usage < 10MB for typical GUI
- **NFR4**: Persistent renderer memory overhead < 50MB

### Error Handling
- **ERR1**: Graceful fallback if WebGPU initialization fails
- **ERR2**: Report clear error messages for shader compilation failures
- **ERR3**: Handle window resize without crashing or corrupting state
- **ERR4**: Arena allocation failures handled gracefully with error codes

### Compatibility
- **NFR5**: Support Windows, macOS, and Linux platforms
- **NFR6**: Work with integrated and discrete GPUs
- **NFR7**: Compatible with Valgrind build configuration

### Integration Requirements
- **INT1**: Use Samrena arenas for all memory allocation
- **INT2**: Follow Ptah library structure (include/, src/, test/)
- **INT3**: Integrate with Ptah CMake build system using ptah_add_library()
- **INT4**: Include Apache License 2.0 header in all source files
- **INT5**: Support Ptah's hexagonal architecture patterns

## Domain Model
- **PtahRenderer**: Main context managing WebGPU device, SDL window, and arena
- **PtahFrame**: Per-frame state with temporary arena for render commands
- **PtahRenderCommand**: Clay UI drawing operation with Ptah memory management
- **PtahEventQueue**: SDL event translation using arena allocation
- **PtahTextureCache**: Simple texture storage with manual management
- **PtahShaderSet**: Built-in shaders for common UI elements

## Acceptance Criteria
- Renderer successfully initializes using Samrena arena
- Can render basic Clay UI layouts (text, rectangles, images) with immediate-mode
- All frame allocations use temporary arena that resets each frame
- Responds to window resize by updating viewport
- Processes mouse clicks and keyboard input correctly
- Maintains 60 FPS with 1000 UI elements
- No memory leaks when used with proper arena cleanup
- Builds successfully within Ptah monorepo structure

## Examples
```c
// Create arena and renderer
SamrenaArena* arena = samrena_arena_create(NULL);

PtahRendererConfig config = {
    .window_width = 1280,
    .window_height = 720,
    .window_title = "Ptah GUI App",
    .vsync = true,
    .arena = arena
};

PtahRenderer* renderer = ptah_renderer_create(arena, &config);
if (!renderer) {
    fprintf(stderr, "Failed to create renderer\n");
    return 1;
}

// Load a texture
PtahTextureHandle icon = ptah_renderer_load_texture(renderer, "icon.png");

// Render loop
bool running = true;
while (running) {
    // Process events
    PtahEvent event;
    while (ptah_renderer_poll_event(renderer, &event)) {
        if (event.type == PTAH_EVENT_QUIT) {
            running = false;
        }
        // Pass events to Clay UI
    }
    
    // Begin frame (resets frame arena)
    ptah_renderer_begin_frame(renderer);
    
    // Clay UI immediate-mode layout
    clay_begin_layout();
    
    // Example UI
    CLAY(CLAY_RECTANGLE({ .color = {100, 100, 100, 255} })) {
        CLAY_TEXT("Hello Ptah!", &(ClayTextConfig){ .fontSize = 24 });
    }
    
    ClayRenderCommandArray commands = clay_end_layout();
    
    // Submit commands to renderer
    ptah_renderer_draw_commands(renderer, &commands);
    
    // Present frame
    ptah_renderer_end_frame(renderer);
}

// Cleanup
ptah_renderer_unload_texture(renderer, icon);
ptah_renderer_destroy(renderer);
samrena_arena_destroy(arena);
```

## Dependencies
- **samrena**: Memory arena management (required)
- **Clay UI**: Layout engine (external)
- **SDL3**: Window and input handling (external)
- **webgpu-native**: WebGPU implementation (external)
- **stb_truetype**: Font rasterization (vendored or external)

## Built-in Shader Details

### Shader Types
1. **Basic Shape Shader**: Rectangles with solid colors
   - Uniforms: MVP matrix, color
   - Features: Solid fill

2. **Rounded Rectangle Shader**: Rectangles with rounded corners
   - Uniforms: MVP matrix, color, corner radius
   - Features: Anti-aliased edges using SDF

3. **Text Shader**: Glyph rendering from texture atlas
   - Uniforms: MVP matrix, texture sampler, text color
   - Features: Alpha blending for anti-aliasing

4. **Image Shader**: Texture rendering with optional tinting
   - Uniforms: MVP matrix, texture sampler, tint color
   - Features: Bilinear filtering, alpha blending

### Shader Management
- All shaders compiled at initialization
- Stored as WGSL source strings in the binary
- Pipeline state objects created for each shader type
- Automatic shader selection based on render command type

## Library Structure
```
libs/ptah_renderer/
├── CMakeLists.txt
├── include/
│   └── ptah_renderer.h          # Public API
├── src/
│   ├── renderer.c               # Core renderer implementation
│   ├── shaders.c                # Built-in shader management
│   ├── text.c                   # Text rendering subsystem
│   ├── texture.c                # Texture management
│   └── shaders/                 # WGSL shader sources
│       ├── basic.wgsl
│       ├── rounded_rect.wgsl
│       ├── text.wgsl
│       └── image.wgsl
└── test/
    ├── test_renderer_basic.c    # Basic rendering tests
    ├── test_text_rendering.c    # Text system tests
    └── test_integration.c       # Full integration tests
```

## Future Considerations (Not in Scope)
- Custom shader support
- Render target textures
- Advanced blend modes
- Gradient fills
- Shadow/glow effects
- Animation system integration
- Retained-mode optimizations
- Multi-window support

These are documented to guide architectural decisions but will not be implemented in the initial version.
