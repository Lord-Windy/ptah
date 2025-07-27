#!/bin/bash

# Script to set up LSP support for clangd

echo "Setting up LSP support for clangd..."

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Generate compile_commands.json
echo "Generating compile_commands.json..."
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cd ..

# Create symlink to compile_commands.json in project root for clangd
echo "Creating symlink for clangd..."
ln -sf build/compile_commands.json compile_commands.json

echo "LSP setup complete! You can now use clangd with your editor."
echo "The compile_commands.json file is located at: $(pwd)/compile_commands.json"