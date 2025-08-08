# Learning Project: Cross-Platform GUI Framework with wgpu & clay

## Project Overview

### Description

Build a modular, cross-platform GUI application framework using wgpu for
rendering and clay for layout management, implementing the MVVM
(Model-View-ViewModel) architecture pattern. This project focuses on creating
a reusable foundation that can be controlled programmatically by external
applications.

### Learning Objectives
- [ ] Master wgpu's rendering pipeline and shader system
- [ ] Understand clay's immediate-mode layout approach
- [ ] Implement proper MVVM separation of concerns
- [ ] Design clean APIs for dynamic UI construction
- [ ] Learn cross-platform considerations for desktop and mobile
- [ ] Practice incremental architecture development
- [ ] Build debugging and profiling capabilities into the framework

## Prerequisites & Setup

### Required Knowledge
- Intermediate Rust programming (ownership, traits, lifetimes)
- Basic understanding of graphics pipelines
- Familiarity with event-driven programming
- Basic linear algebra (matrices, vectors for transformations)

### Development Environment
```toml
# Cargo.toml dependencies
[dependencies]
wgpu = "0.19"
winit = "0.29"  # Window management
clay = { git = "https://github.com/nicbarker/clay" }
bytemuck = "1.14"  # For shader data
env_logger = "0.11"
log = "0.4"
pollster = "0.3"  # Async runtime for wgpu

# Platform-specific dependencies
[target.'cfg(target_os = "android")'.dependencies]
android-activity = "0.5"

[target.'cfg(target_os = "ios")'.dependencies]
objc = "0.2"
```

### Project Structure
```
ptah/
├── src/
│   ├── main.rs              # Application entry point
│   ├── core/
│   │   ├── mod.rs
│   │   ├── renderer.rs      # wgpu rendering pipeline
│   │   └── window.rs        # Window management
│   ├── mvvm/
│   │   ├── mod.rs
│   │   ├── model.rs         # Data models
│   │   ├── view.rs          # View components
│   │   └── view_model.rs    # Business logic & bindings
│   ├── ui/
│   │   ├── mod.rs
│   │   ├── layout.rs        # clay integration
│   │   ├── widgets/         # Reusable UI components
│   │   └── theme.rs         # Styling system
│   └── api/
│       ├── mod.rs
│       └── builder.rs       # External API for UI construction
├── shaders/
│   ├── basic.wgsl           # Basic vertex/fragment shaders
│   └── ui.wgsl              # UI-specific shaders
└── examples/
    ├── basic_window.rs
    ├── mvvm_counter.rs
    └── dynamic_ui.rs
```

## Incremental Milestones

### Milestone 1: Basic Window & wgpu Setup
**Duration:** 2-3 days

#### Objectives
- [ ] Create a basic window using winit
- [ ] Initialize wgpu instance, device, and surface
- [ ] Render a colored triangle to verify pipeline
- [ ] Handle window events (resize, close)

#### Implementation Steps
1. **Window Creation**
   ```rust
   // src/core/window.rs
   pub struct Window {
       event_loop: EventLoop<()>,
       window: winit::window::Window,
   }
   ```

2. **wgpu Initialization**
   ```rust
   // src/core/renderer.rs
   pub struct Renderer {
       surface: wgpu::Surface,
       device: wgpu::Device,
       queue: wgpu::Queue,
       config: wgpu::SurfaceConfiguration,
   }
   ```

3. **Basic Render Pipeline**
   - Create shader module from WGSL
   - Define vertex buffer layout
   - Create render pipeline
   - Implement render loop

#### Key Concepts
- GPU device and queue management
- Surface configuration for different platforms
- Render pipeline stages
- Vertex buffer management

#### Validation Criteria
- [ ] Window appears on screen
- [ ] Triangle renders with correct color
- [ ] Window can be resized without crashing
- [ ] Clean shutdown on window close

### Milestone 2: clay Integration for Layout
**Duration:** 3-4 days

#### Objectives
- [ ] Integrate clay layout engine
- [ ] Render rectangles based on clay layout
- [ ] Implement basic UI primitives (box, text area)
- [ ] Handle mouse/touch input for layout

#### Implementation Steps
1. **clay Setup**
   ```rust
   // src/ui/layout.rs
   pub struct LayoutEngine {
       clay_context: Clay,
       element_tree: Vec<Element>,
   }
   ```

2. **Layout to Vertex Conversion**
   - Convert clay rectangles to vertex data
   - Generate index buffers for efficient rendering
   - Implement batching for performance

3. **Input Handling**
   - Map winit events to clay input
   - Implement hit testing
   - Handle hover and click states

#### Key Concepts
- Immediate mode vs retained mode UI
- Layout algorithms (flexbox-style)
- Vertex generation from layout data
- Input coordinate transformation

#### Validation Criteria
- [ ] Render nested boxes with clay layout
- [ ] Correct layout on window resize
- [ ] Mouse hover highlights elements
- [ ] Click events detected on correct elements

### Milestone 3: MVVM Architecture Foundation
**Duration:** 4-5 days

#### Objectives
- [ ] Implement Model layer with observable properties
- [ ] Create ViewModel with data binding
- [ ] Build View components that react to changes
- [ ] Establish command pattern for user actions

#### Implementation Steps
1. **Observable Model**
   ```rust
   // src/mvvm/model.rs
   pub trait Observable<T> {
       fn subscribe(&mut self, callback: Box<dyn Fn(&T)>);
       fn notify(&self);
   }
   
   pub struct Property<T> {
       value: T,
       observers: Vec<Box<dyn Fn(&T)>>,
   }
   ```

2. **ViewModel Layer**
   ```rust
   // src/mvvm/view_model.rs
   pub trait ViewModel {
       type Model;
       fn bind_model(&mut self, model: Self::Model);
       fn handle_command(&mut self, cmd: Command);
   }
   ```

3. **View Binding**
   ```rust
   // src/mvvm/view.rs
   pub trait View {
       type ViewModel;
       fn bind(&mut self, vm: &Self::ViewModel);
       fn render(&self, layout: &mut LayoutEngine);
   }
   ```

#### Key Concepts
- Observer pattern implementation
- Data binding strategies
- Command pattern for actions
- Separation of concerns
- Reactive programming principles

#### Validation Criteria
- [ ] Model changes trigger view updates
- [ ] Commands execute business logic
- [ ] No direct model access from views
- [ ] Memory-safe observer management

### Milestone 4: Widget System & Styling
**Duration:** 3-4 days

#### Objectives
- [ ] Create reusable widget components
- [ ] Implement theming system
- [ ] Build common widgets (button, label, input)
- [ ] Support custom widget creation

#### Implementation Steps
1. **Widget Trait**
   ```rust
   // src/ui/widgets/mod.rs
   pub trait Widget {
       fn layout(&self, constraints: Constraints) -> Size;
       fn paint(&self, canvas: &mut Canvas);
       fn handle_event(&mut self, event: Event) -> bool;
   }
   ```

2. **Built-in Widgets**
   - Button with states (normal, hover, pressed)
   - Label with text rendering
   - TextInput with cursor and selection
   - Container for composition

3. **Theme System**
   ```rust
   // src/ui/theme.rs
   pub struct Theme {
       colors: HashMap<String, Color>,
       fonts: HashMap<String, Font>,
       spacing: Spacing,
   }
   ```

#### Key Concepts
- Composite pattern for widgets
- Theme inheritance
- Event bubbling
- Custom painting APIs

#### Validation Criteria
- [ ] Widgets render correctly
- [ ] Theme changes apply immediately
- [ ] Custom widgets can be created
- [ ] Events propagate correctly

### Milestone 5: External API & Dynamic UI
**Duration:** 4-5 days

#### Objectives
- [ ] Design builder API for external control
- [ ] Implement serializable UI descriptions
- [ ] Create dynamic UI update mechanism
- [ ] Build example applications using the API

#### Implementation Steps
1. **Builder API**
   ```rust
   // src/api/builder.rs
   pub struct UIBuilder {
       pub fn column(&mut self) -> &mut Self;
       pub fn row(&mut self) -> &mut Self;
       pub fn button(&mut self, text: &str) -> &mut Self;
       pub fn on_click(&mut self, handler: fn()) -> &mut Self;
       pub fn build(&self) -> UITree;
   }
   ```

2. **Declarative UI Format**
   ```rust
   // Support for JSON/TOML UI descriptions
   #[derive(Serialize, Deserialize)]
   pub struct UIDescription {
       root: Element,
       bindings: Vec<Binding>,
       handlers: Vec<Handler>,
   }
   ```

3. **Hot Reload Support**
   - File watcher for UI descriptions
   - Runtime UI tree updates
   - State preservation during reload

#### Key Concepts
- Fluent interface design
- Serialization strategies
- Dynamic dispatch for handlers
- State management across reloads

#### Validation Criteria
- [ ] UI can be built programmatically
- [ ] JSON/TOML descriptions work
- [ ] Hot reload preserves state
- [ ] External apps can control UI

### Milestone 6: Performance & Debugging Tools
**Duration:** 3-4 days

#### Objectives
- [ ] Implement render performance metrics
- [ ] Add visual debugging overlays
- [ ] Create profiling integration
- [ ] Build inspector tool for UI tree

#### Implementation Steps
1. **Performance Metrics**
   ```rust
   pub struct Metrics {
       frame_time: Duration,
       draw_calls: usize,
       vertex_count: usize,
       layout_time: Duration,
   }
   ```

2. **Debug Overlays**
   - Wireframe mode for layout bounds
   - Render order visualization
   - Event flow visualization
   - Performance graphs

3. **Inspector Tool**
   - Live UI tree view
   - Property inspection
   - Event monitoring
   - State snapshots

#### Key Concepts
- Performance profiling techniques
- Debug rendering strategies
- Instrumentation approaches
- Real-time monitoring

#### Validation Criteria
- [ ] FPS counter accurate
- [ ] Debug overlays toggle correctly
- [ ] Inspector shows live updates
- [ ] No performance impact when disabled

### Milestone 7: Cross-Platform Preparation
**Duration:** 5-6 days

#### Objectives
- [ ] Abstract platform-specific code
- [ ] Implement touch input handling
- [ ] Add platform-specific build configurations
- [ ] Create deployment examples

#### Implementation Steps
1. **Platform Abstraction Layer**
   ```rust
   // src/core/platform.rs
   pub trait Platform {
       fn create_window(&self) -> Window;
       fn handle_lifecycle(&mut self, event: LifecycleEvent);
   }
   ```

2. **Touch Input Support**
   - Multi-touch tracking
   - Gesture recognition
   - Touch-to-mouse mapping

3. **Build Configurations**
   ```toml
   # Platform-specific features
   [features]
   ios = ["dep:objc"]
   android = ["dep:android-activity"]
   ```

#### Key Concepts
- Platform abstraction patterns
- Touch vs mouse input models
- Mobile lifecycle management
- Resource bundling

#### Validation Criteria
- [ ] Builds for desktop platforms
- [ ] Touch simulation works on desktop
- [ ] Platform layer properly isolated
- [ ] Example apps run on each platform

## Resources

### wgpu Learning
- [Learn wgpu](https://sotrh.github.io/learn-wgpu/) - Comprehensive tutorial
- [wgpu Examples](https://github.com/gfx-rs/wgpu/tree/master/examples) - Official examples
- [WebGPU Fundamentals](https://webgpufundamentals.org/) - Core concepts

### clay Resources
- [clay GitHub](https://github.com/nicbarker/clay) - Official repository
- Study the C examples and port concepts to Rust

### MVVM in Rust
- [Druid Architecture](https://github.com/linebender/druid) - Reference implementation
- [The Elm Architecture](https://guide.elm-lang.org/architecture/) - Inspiration for Rust MVVM

### General GUI Programming
- [Immediate Mode GUIs](https://github.com/ocornut/imgui/wiki) - Concepts applicable to clay
- [Flutter's Rendering Pipeline](https://flutter.dev/docs/resources/rendering) - Modern UI architecture

## Common Pitfalls & Debugging Tips

### wgpu Pitfalls
1. **Validation Errors**
   - Enable validation layers in debug builds
   - Check WebGPU limits for target devices
   - Ensure buffer sizes match shader expectations

2. **Performance Issues**
   - Minimize state changes between draw calls
   - Use instancing for repeated elements
   - Profile with RenderDoc or similar tools

### clay Integration Challenges
1. **Layout Conflicts**
   - Verify constraint propagation
   - Check for circular dependencies
   - Use debug visualizations for layout bounds

### MVVM Gotchas
1. **Memory Leaks**
   - Weak references for circular dependencies
   - Proper cleanup in drop implementations
   - Use Arc/Rc judiciously

2. **Update Storms**
   - Batch updates when possible
   - Implement change detection
   - Use dirty flags for render optimization

### Debugging Tools
```rust
// Enable debug logging
RUST_LOG=debug cargo run

// GPU debugging
WGPU_BACKEND=vulkan cargo run  # For RenderDoc
```

## Extension Ideas

### After Prototype Completion
1. **Advanced Rendering**
   - Custom shaders for effects
   - Text rendering with SDF fonts
   - Animations and transitions
   - Shadow and blur effects

2. **Enhanced Architecture**
   - Plugin system for extensions
   - Scripting language bindings
   - Network-transparent UI
   - Collaborative editing support

3. **Specialized Applications**
   - Code editor with syntax highlighting
   - Data visualization dashboard
   - Game UI framework
   - Design tool prototype

4. **Performance Optimizations**
   - GPU-accelerated layout
   - Texture atlasing
   - Render caching
   - Parallel rendering

5. **Developer Experience**
   - Visual UI designer
   - Live coding environment
   - Component library
   - Documentation generator

## Progress Tracking

### Phase 1: Foundation (Milestones 1-3)
- [ ] Basic window and rendering
- [ ] clay integration
- [ ] MVVM architecture

### Phase 2: Features (Milestones 4-5)
- [ ] Widget system
- [ ] External API
- [ ] Dynamic UI

### Phase 3: Polish (Milestones 6-7)
- [ ] Performance tools
- [ ] Cross-platform support
- [ ] Production readiness

### Learning Checkpoints
- [ ] Can explain wgpu pipeline from memory
- [ ] Understand clay's layout algorithm
- [ ] Can implement MVVM from scratch
- [ ] Ready to build production apps

## Notes Section

### Architecture Decisions Log
_Document key decisions and their rationale as you progress_

### Performance Benchmarks
_Track frame times and memory usage at each milestone_

### Platform-Specific Notes
_Record platform quirks and solutions_

---

**Remember:** This is a learning project. Prioritize understanding over perfection. Each milestone should produce working code that demonstrates the concepts, even if not production-ready. Refactor and improve as understanding deepens.
