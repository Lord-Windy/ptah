# C Code Linting Setup

This project now includes a C code linter setup using `clang-format` to ensure consistent code formatting.

## Configuration

The `.clang-format` file in the project root defines the formatting rules:
- Based on LLVM style
- 4-space indentation
- No tabs
- Column limit of 100 characters
- Other formatting preferences

## Usage

You can lint/format your C code in two ways:

### 1. Using the shell script (simplest)

```bash
# Check for formatting issues
./lint.sh

# Automatically fix formatting issues
./lint.sh format
```

### 2. Using CMake targets

```bash
# Check for formatting issues
cd build
make lint

# Automatically fix formatting issues
cd build
make format
```

## What's been done

1. Created a `.clang-format` configuration file
2. Added a `lint.sh` shell script for easy linting
3. Added `lint` and `format` targets to the CMake build system
4. Updated the README.md with instructions
5. Formatted all existing C code to match the style

The linter will help maintain consistent code style while respecting your preference for snake_case naming and keeping semicolons as they are.