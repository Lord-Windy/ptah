This file provides guidance to opencode when working with code in this repository.

## Project Overview

This is a Rust workspace monorepo named "ptah" using Cargo's workspace feature. The project is organized with:
- `lib/`: Directory for library crates (reusable components)
  - `lib/ptah-lib`: A library crate providing core functionality
  - `lib/cartouche`: Another library crate
- `app/`: Directory for executable crates (applications)
  - `app/ptah-app`: An application crate that depends on ptah-lib

## Common Development Commands

### Build Commands
```bash
# Build the entire workspace
cargo build

# Build in release mode
cargo build --release

# Build a specific package
cargo build -p ptah-app
cargo build -p ptah-lib
cargo build -p cartouche
```

### Running the Application
```bash
# Run the main application
cargo run -p ptah-app

# Run with release optimizations
cargo run --release -p ptah-app
```

### Testing
```bash
# Run all tests in the workspace
cargo test

# Run tests for a specific package
cargo test -p ptah-lib
cargo test -p cartouche
cargo test -p ptah-app

# Run tests with output displayed
cargo test -- --nocapture
```

### Code Quality
```bash
# Format all code in the workspace
cargo fmt

# Check formatting without modifying files
cargo fmt -- --check

# Run clippy linter
cargo clippy

# Run clippy with stricter settings
cargo clippy -- -W clippy::pedantic
```

### Documentation
```bash
# Generate and open documentation
cargo doc --open

# Generate documentation without dependencies
cargo doc --no-deps
```

## Architecture

The project follows a simple monorepo structure with clear separation between libraries and executables:

### Directory Structure
- **`lib/`**: Contains library crates (reusable components)
  - **ptah-lib** (`lib/ptah-lib/`): Core library providing shared functionality
  - **cartouche** (`lib/cartouche/`): Additional library crate

- **`app/`**: Contains executable crates (applications)
  - **ptah-app** (`app/ptah-app/`): Main application
    - Depends on ptah-lib
    - Entry point at `src/main.rs`

The workspace is configured in the root `Cargo.toml` with shared package metadata that all member crates inherit.