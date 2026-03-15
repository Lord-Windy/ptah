# AGENTS.md — Samtasker Agent Template

This file instructs AI coding agents on how to use `samtasker` (`st`) to manage
tasks within this project.

## Project setup

This git repository is the project. Use the repository name (typically
UPPERCASE, alphanumeric only) as the project identifier when creating tasks.
Assume `samtasker` is already initialized and configured by the user.

## Project Overview

Ptah is a CMake-based C monorepo with Bazel-like modularity. The repository
contains two main libraries (`samrena` and `samdata`) and example applications
showcasing their integration. The project uses C11 standard and follows
a structured approach to library development with hexagonal architecture
patterns.

### Core Libraries

**samrena** - Memory Arena Management
- Provides efficient memory allocation with multiple strategies (default,
  chained, virtual)
- Implements hexagonal architecture with adapters for different memory
  management approaches
- Cross-platform virtual memory support (Windows, macOS, Linux)
- Includes SamrenaVector for dynamic arrays using arena allocation

**samdata** - Data Structures Collection
- **SamHashMap**: Hash map implementation with multiple hash functions (DJB2,
  FNV1A, Murmur3)
- **SamSet**: Set data structure for unique elements
- **SamHash**: Hash function collection (DJB2, FNV1A, Murmur3)
- All structures are arena-backed using samrena for memory management

### Dependency Graph
```
samdata → samrena
demo → samrena, samdata
examples → samrena, samdata
```

### Dependencies

The project uses the following external dependencies:
- **libpq** (PostgreSQL client library): Required for database operations
- **libcurl**: For HTTP requests (used in samspacetrader)
- **cjson**: JSON parsing library (used in samspacetrader)

## Build System

### Common Commands

**Build the project:**
```bash
mkdir build
cd build
cmake ..
make
```

**Run all tests:**
```bash
cd build
ctest
```

**Run tests with verbose output:**
```bash
cd build
ctest --output-on-failure
```

**Run a single test:**
```bash
cd build
ctest -R test_name  # e.g., ctest -R samrena_basic
```

**Code formatting:**
```bash
# Check formatting
make lint

# Fix formatting
make format

# Alternative (from root)
./lint.sh
```

**LSP setup for IDE support:**
```bash
./setup_lsp.sh
```

**Valgrind memory testing:**
```bash
./valgrind_test.sh
```

**Build with different configurations:**
```bash
# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..

# Debug build (default)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Valgrind build (restricted instruction set)
cmake -DCMAKE_BUILD_TYPE=Valgrind ..

# Shared libraries
cmake -DBUILD_SHARED_LIBS=ON ..

# Disable tests
cmake -DBUILD_TESTING=OFF ..

# Enable Valgrind tests
cmake -DENABLE_VALGRIND_TESTS=ON ..
```

### Custom CMake Functions

The build system provides two helper functions for consistent target creation:

- `ptah_add_library()`: Creates libraries with proper include directories
  and installation rules
- `ptah_add_executable()`: Creates executables with dependency management

### Automatic Discovery

The build system automatically discovers any directory containing
a `CMakeLists.txt` file in:
- `libs/` - Libraries
- `apps/` - Applications  
- `tools/` - Development tools
- `tests/` - Test suites (when `BUILD_TESTING=ON`)

## Development Guidelines

### Adding New Components

**New Library:**
Create directory under `libs/` with `CMakeLists.txt`:
```cmake
ptah_add_library(mylib
    SOURCES
        src/mylib.c
    PUBLIC_HEADERS
        include/mylib.h
    DEPENDENCIES
        # other libraries
        PostgreSQL::PostgreSQL  # if database functionality needed
)
```

**New Application:**
Create directory under `apps/` with `CMakeLists.txt`:
```cmake
ptah_add_executable(myapp
    SOURCES
        src/main.c
    DEPENDENCIES
        mylib
        PostgreSQL::PostgreSQL  # if database functionality needed
)
```

### License Requirements

**All source files must include the Apache License 2.0 header with
copyright to Samuel "Lord-Windy" Brown.**

For C/C++ files (.c, .h):
```c
/*
 * Copyright 2026 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

For CMake files:
```cmake
# Copyright 2026 Samuel "Lord-Windy" Brown
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
```

### Code Structure

Each library follows the standard layout:
```
libs/library_name/
├── CMakeLists.txt
├── include/
│   └── library_name.h
├── src/
│   └── implementation files
└── test/
    └── test files
```

## Testing

Tests are automatically built when `BUILD_TESTING=ON` (default). Each
library can have its own test suite in its `test/` directory. Test
executables are created using the same `ptah_add_executable()` helper
function.

### Test Organization

- **Samrena tests**: Basic arena, vector operations, performance, type
  safety, adapters
- **Samdata tests**: SamHashMap (hash map) and SamSet (set) functionality,
  collision handling, resizing

### Platform-Specific Notes

The project includes sophisticated platform detection for virtual memory
support. The Valgrind build type restricts CPU instructions to ensure
compatibility with Valgrind's instruction set emulation.

## Creating tickets

### Via CLI flags

```bash
st add --project PTAH --summary "Implement auth middleware" \
  --description "Add JWT validation to all API routes" \
  --labels backend,security
```

### Via JSON (single)

```bash
echo '{"project":"PTAH","summary":"Implement auth middleware","description":"Add JWT validation to all API routes","labels":["backend","security"]}' | st add --json
```

### Via JSON (batch)

```bash
cat <<'EOF' | st add --json
[
  {"project":"PTAH","summary":"Design database schema","labels":["backend"]},
  {"project":"PTAH","summary":"Build API endpoints","labels":["backend"]},
  {"project":"PTAH","summary":"Create frontend components","labels":["frontend"]}
]
EOF
# Output: ["a1b2c3d4", "b2c3d4e5", "c3d4e5f6"] (Array of created task IDs)
```

## Creating tickets with dependencies

Dependencies ensure tickets are worked on in the correct order. A ticket
with unresolved dependencies will not appear as ready.

```bash
# Create the first ticket
st add --project PTAH --summary "Design database schema"
# Output: Created task: a1b2c3d4

# Create a dependent ticket
st add --project PTAH --summary "Build API endpoints" --depends-on a1b2c3d4
```

Or via JSON:

```bash
cat <<'EOF' | st add --json
[
  {"project":"PTAH","summary":"Design database schema"},
  {"project":"PTAH","summary":"Build API endpoints","dependencies":["a1b2c3d4"]}
]
EOF
```

You can also add dependencies after creation:

```bash
st depend b7e2d4a1 --on a1b2c3d4
```

## Editing tickets

### Via CLI flags

```bash
st edit a1b2c3d4 --summary "Updated summary"
st edit a1b2c3d4 --description "More detailed description"
st edit a1b2c3d4 --add-labels urgent
st edit a1b2c3d4 --remove-labels low-priority
```

### Via JSON

```bash
echo '{"key":"a1b2c3d4","summary":"Updated summary","labels":["urgent","backend"]}' | st edit --json
```

Note: when editing via JSON, `labels` and `dependencies` replace all
existing values (they are not additive).

## Finding tickets ready to work on

```bash
st list --ready --project PTAH
```

*Note: For AI agents, it is highly recommended to append `--json` to `list` and `show` commands for reliable parsing.*
```bash
st list --ready --project PTAH --json
```

This shows only tickets that:
- Belong to the PTAH project
- Have status `ready`
- Have all dependencies resolved (all dependencies are `done` or `cancelled`)

You can combine filters:

```bash
st list --ready --project PTAH --label backend
```

To see full details of a specific ticket:

```bash
st show a1b2c3d4

# Or for reliable parsing:
st show a1b2c3d4 --json
```

## Working on a ticket

When you pick up a ticket, move it to `in-progress`:

```bash
st status a1b2c3d4 in-progress
```

## Completing a ticket

When the implementation is done:

1. **Commit and push the work** to preserve it for review:

```bash
git add <files>
git commit -m "Implement feature X"
git push
```

2. **Set the ticket as done** once pushed:

```bash
st status a1b2c3d4 done
```

Always commit and push before marking a ticket as done. The work must be
preserved on the remote for review. Do not mark a ticket done until the
code is pushed.

When a ticket is marked done, any dependent tickets that have all their
dependencies resolved will automatically be promoted from `backlog` to
`ready`.

## Status transitions

```
backlog → ready → in-progress → done
                       ↓
                    blocked → in-progress
                            → ready

Any status → cancelled
```

`done` and `cancelled` are terminal — no further transitions are allowed.

## Quick reference

| Action | Command |
|---|---|
| Create a ticket | `st add --project PTAH --summary "..."` |
| Create with dependency | `st add --project PTAH --summary "..." --depends-on KEY` |
| List ready tickets | `st list --ready --project PTAH [--json]` |
| Show ticket details | `st show KEY [--json]` |
| Start working | `st status KEY in-progress` |
| Mark done | `st status KEY done` |
| Edit summary | `st edit KEY --summary "..."` |
| Add dependency | `st depend KEY --on OTHER_KEY` |
| Remove dependency | `st undepend KEY --on OTHER_KEY` |
| Batch create (JSON) | `cat tasks.json \| st add --json` |
| Export all tickets | `st export --json` |
