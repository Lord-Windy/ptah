# OpenCode Configuration for Ptah

## Build/Test/Lint Commands
- **Build**: `cargo build` (workspace root)
- **Test all**: `cargo test` (workspace root)
- **Test single**: `cargo test test_name` (runs specific test by name)
- **Test package**: `cargo test -p ptah-lib` or `cargo test -p ptah-app`
- **Check/Lint**: `cargo check` and `cargo clippy`
- **Format**: `cargo fmt`
- **Run app**: `cargo run -p ptah-app`

## Code Style Guidelines
- **License**: Apache 2.0 header required in all source files
- **Edition**: Rust 2021, minimum version 1.70
- **Imports**: Use `use` statements, group std/external/local imports
- **Naming**: snake_case for functions/variables, PascalCase for types
- **Error handling**: Use `Result<T, E>` for fallible operations
- **Tests**: Place in `#[cfg(test)]` mod tests with descriptive names
- **Documentation**: Use `///` for public APIs
- **Formatting**: Use `cargo fmt` (rustfmt) for consistent style
- **Workspace**: Monorepo structure with lib/ and app/ directories