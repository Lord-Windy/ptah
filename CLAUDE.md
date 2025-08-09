# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Structure

This is a Zig monorepo following strict Hexagonal Architecture principles.

### Monorepo Organization
```
apps/           # Applications
libs/           # Reusable libraries
build.zig       # Root build configuration
```

- **apps/**: Contains standalone applications, each with its own `build.zig`
- **libs/**: Contains reusable libraries, each with its own `build.zig`
- Libraries are imported as modules in the root `build.zig` and made available to apps

## Commands

### Build Commands
```bash
# Build entire project
zig build

# Run all tests
zig build test

# Run a specific app (replace app_name)
zig build run

# Run with arguments
zig build run -- arg1 arg2
```

### Library Development
```bash
cd libs/<library_name>
zig build test
```

### Application Development
```bash
cd apps/<app_name>
zig build test
zig build run
```

## Hexagonal Architecture

All code must follow Hexagonal Architecture (Ports and Adapters) principles:

### Domain Core
- Located in `src/domain/`
- Pure business logic with no external dependencies
- Contains core entities, value objects, and business rules

### Ports
- Located in `src/ports/`
- Interfaces defining how the domain interacts with the outside world
- Input ports (use cases) and output ports (driven adapters)

### Adapters
- Located in `src/adapters/`
- Concrete implementations of ports
- Primary adapters (controllers, CLI, API)
- Secondary adapters (repositories, external services)

### Directory Structure for New Components
```
src/
├── domain/        # Core business logic
├── ports/         # Interface definitions
├── adapters/      # External implementations
│   ├── primary/   # Driving adapters (UI, API)
│   └── secondary/ # Driven adapters (DB, services)
└── main.zig       # Application entry point
```

## Code Requirements

### License Header
All source files MUST include the following license header:
```zig
// Copyright (c) Samuel "Lord Windy" Brown
// Licensed under the terms in LICENSE
```

### Zig Code Style
- Use snake_case for variables and functions
- Use PascalCase for types and structs
- Keep functions small and focused
- Prefer composition over inheritance
- Use comptime where appropriate

## Testing

- Unit tests should be colocated with source files
- Integration tests go in the `test/` directory
- Test files should be named `*_test.zig`
- Use descriptive test names that explain what is being tested