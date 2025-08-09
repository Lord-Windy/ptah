# Zig Monorepo Example

This is an example Zig monorepo structured following Hexagonal Architecture principles.

## Structure

```
.
├── apps/
│   └── example_app/
│       ├── src/
│       │   ├── main.zig
│       │   └── main_test.zig
│       └── build.zig
├── libs/
│   └── example_lib/
│       ├── src/
│       │   ├── main.zig
│       │   └── main_test.zig
│       └── build.zig
├── build.zig
└── build.zig.zon
```

## Hexagonal Architecture

This example follows Hexagonal Architecture (also known as Ports and Adapters) where:

- **Core Domain**: Business logic is kept separate from external concerns
- **Ports**: Interfaces that define how the application core interacts with the outside world
- **Adapters**: Implementations of ports that connect to specific external systems

In our example:
- The `Calculator` struct represents the core domain logic
- The `InputPort` and `OutputPort` structs define interfaces (ports)
- The `ConsoleAdapter` and `TestAdapter` structs are concrete implementations (adapters)

## Building and Running

To build the entire project:

```bash
zig build
```

To run the example application:

```bash
zig build run
```

To run all tests:

```bash
zig build test
```

To build and run the app with specific arguments:

```bash
zig build run -- arg1 arg2
```

## Library Development

To work specifically with the library:

```bash
cd libs/example_lib
zig build test
```

## Application Development

To work specifically with the application:

```bash
cd apps/example_app
zig build test
zig build run
```
