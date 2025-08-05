# Cartouche - Immediate Mode GUI System

Cartouche is a complete GUI system built using wgpu for rendering and Clay UI for the immediate mode interface. It provides a clean external API for applications to present formatted content and handle user interactions while abstracting the internal rendering complexity.

## Goals

1. Provide a complete GUI system with a clean external API for applications
2. Abstract internal complexity of wgpu rendering and Clay UI
3. Support formatted content presentation and user input handling
4. Enable easy integration with host applications
5. Follow modular architecture principles for maintainability
6. Optimize for performance in immediate mode GUI contexts

## Architecture Overview

### Core Components

1. **GUI Context** - Main entry point that manages the complete GUI system
2. **Renderer** - Handles wgpu context and rendering pipeline
3. **Input Handler** - Processes user input events and maps to GUI interactions
4. **Layout Engine** - Manages element positioning and sizing (using Clay UI)
5. **Widget Manager** - Manages GUI widgets and their states
6. **Resource Manager** - Manages textures, fonts, and other resources
7. **Event System** - Handles communication between components and external applications

### External Interface

```
Host Application
      |
      v
  Cartouche GUI
      |
  +---+---+
  |       |
  |  API  |<---> Formatted Content
  |       |
  +---+---+
      |
  +---+---+
  |       |
  | Input |<---> User Interactions
  |       |
  +---+---+
      |
      v
   Internal
   Components
```

### Data Flow

```
Host Application
      |
      v
Formatted Content
      |
      v
GUI Context (Cartouche)
      |
+-----+-----+
|           |
v           v
Clay UI --> Renderer --> Wgpu --> GPU
  |
  v
Input Handler --> Host Application (Events/Callbacks)
```

## Implementation Plan

### Phase 1: Foundation

1. Setup wgpu context and basic windowing (using winit)
2. Create core GUI Context struct with initialization
3. Implement basic rendering loop with Clay UI integration
4. Setup input event handling system

### Phase 2: Content Presentation

1. Define data structures for formatted content
2. Implement content update API for host applications
3. Create basic widget system (labels, buttons, etc.)
4. Implement layout engine using Clay UI

### Phase 3: User Input Handling

1. Implement input event processing
2. Create event mapping to GUI interactions
3. Add callback system for user interactions
4. Implement state management for widgets

### Phase 4: Resource Management

1. Create resource manager for textures and fonts
2. Implement font rendering/text display system
3. Add support for custom widgets and styling
4. Implement proper resource cleanup

### Phase 5: API Refinement & Examples

1. Refine external API based on usage patterns
2. Create comprehensive example applications
3. Implement performance optimizations
4. Documentation and usage examples

## Interface Design

### Public API

```rust
// Main GUI Context interface
pub struct CartoucheGUI {
    // Internal components...
}

impl CartoucheGUI {
    pub fn new(window: &Window) -> Result<Self, GUIError> { /* ... */ }
    
    // Content presentation
    pub fn set_content(&mut self, content: FormattedContent) { /* ... */ }
    pub fn update_content(&mut self, content: FormattedContent) { /* ... */ }
    
    // Render lifecycle
    pub fn begin_frame(&mut self) -> Result<(), GUIError> { /* ... */ }
    pub fn end_frame(&mut self) -> Result<(), GUIError> { /* ... */ }
    
    // Input handling
    pub fn handle_input(&mut self, event: InputEvent) -> Vec<GUIEvent> { /* ... */ }
    
    // Configuration
    pub fn resize(&mut self, width: u32, height: u32) { /* ... */ }
    pub fn set_theme(&mut self, theme: Theme) { /* ... */ }
}

// Supporting types
pub struct FormattedContent { /* ... */ }
pub struct InputEvent { /* ... */ }
pub struct GUIEvent { /* ... */ }
pub struct Theme { /* ... */ }
pub enum GUIError { /* ... */ }

// Callback system for user interactions
pub trait GUIEventHandler {
    fn on_button_click(&mut self, id: &str);
    fn on_text_input(&mut self, id: &str, text: &str);
    fn on_selection_change(&mut self, id: &str, selected: usize);
    // ... other event handlers
}
```

### Integration Example

```rust
// Example usage in a host application
impl MyApplication {
    pub fn run() -> Result<(), Box<dyn std::error::Error>> {
        let window = create_window();
        let mut gui = CartoucheGUI::new(&window)?;
        
        loop {
            // Handle window events
            let events = get_window_events();
            for event in events {
                match event {
                    WindowEvent::Resize(width, height) => {
                        gui.resize(width, height);
                    }
                    WindowEvent::Input(input_event) => {
                        let gui_events = gui.handle_input(input_event);
                        self.handle_gui_events(gui_events);
                    }
                    // ... other events
                }
            }
            
            // Update content if needed
            if self.content_changed {
                gui.update_content(self.get_formatted_content());
            }
            
            // Render GUI
            gui.begin_frame()?;
            gui.end_frame()?;
        }
    }
    
    fn handle_gui_events(&mut self, events: Vec<GUIEvent>) {
        for event in events {
            match event {
                GUIEvent::ButtonClicked(id) => {
                    // Handle button click
                }
                GUIEvent::TextInput(id, text) => {
                    // Handle text input
                }
                // ... other event handling
            }
        }
    }
}
```

## Technical Considerations

### WGPU Specifics

1. Use modern wgpu 0.19+ for best features and stability
2. Implement proper error handling for GPU operations
3. Use appropriate buffer usage flags for dynamic vertex data
4. Leverage wgpu's bind group system for efficient resource binding

### Performance Considerations

1. Batch similar draw commands together
2. Minimize state changes (shader switches, texture binds)
3. Use appropriate buffer update strategies (write_buffer vs mapping)
4. Consider instancing for repeated geometry

### Extensibility

1. Allow custom shaders while maintaining core functionality
2. Provide hooks for pre/post render operations
3. Support multiple texture formats
4. Allow integration with different windowing systems

## Dependencies

- wgpu 0.19+
- winit (for windowing in examples)
- image (for texture loading)
- rusttype or ab_glyph (for font rendering)
- log (for logging)

## File Structure

```
cartouche/
├── src/
│   ├── lib.rs
│   ├── gui.rs
│   ├── renderer/
│   │   ├── mod.rs
│   │   ├── pipeline.rs
│   │   └── resources.rs
│   ├── input/
│   │   ├── mod.rs
│   │   └── handler.rs
│   ├── layout/
│   │   ├── mod.rs
│   │   └── clay_integration.rs
│   ├── widgets/
│   │   ├── mod.rs
│   │   ├── button.rs
│   │   ├── text.rs
│   │   └── container.rs
│   ├── content/
│   │   ├── mod.rs
│   │   ├── formatter.rs
│   │   └── types.rs
│   └── error.rs
├── examples/
│   ├── simple.rs
│   └── application.rs
├── Cargo.toml
└── README.md
```

## Milestones

1. **Basic GUI System** - Can render formatted text and handle basic input (Week 1)
2. **Content API** - Complete API for content presentation with update mechanism (Week 2)
3. **Widget System** - Basic widgets (buttons, text inputs) with event handling (Week 3)
4. **Layout System** - Working layout engine with Clay UI integration (Week 4)
5. **Complete Integration** - Working example with a complete application (Week 5)
6. **API Refinement** - Polish API based on usage, add documentation (Week 6)

## Testing Strategy

1. Unit tests for individual components (widgets, layout engine, input handler)
2. Integration tests for content presentation and user input handling
3. Visual regression tests comparing expected vs actual rendering
4. Performance benchmarks for UI responsiveness and rendering throughput
5. Example application tests to verify complete integration

## Future Enhancements

1. Support for advanced blending modes
2. Particle system rendering
3. Gradient rendering support
4. SVG path rendering
5. Canvas-style 2D drawing API
