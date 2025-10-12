#!/usr/bin/env bash

# Copyright 2025 Samuel "Lord-Windy" Brown
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

# Script to generate ctags for the Ptah project
# Usage: ./generate_tags.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "Generating ctags for Ptah project..."

# Check if ctags is installed
if ! command -v ctags &> /dev/null; then
    echo "Error: ctags not found. Please install ctags:"
    echo "  Ubuntu/Debian: sudo apt-get install universal-ctags"
    echo "  Arch/Gentoo: sudo pacman -S ctags / sudo emerge ctags"
    echo "  macOS: brew install universal-ctags"
    exit 1
fi

# Check ctags version
CTAGS_VERSION=$(ctags --version 2>&1 | head -n 1)
echo "Using: $CTAGS_VERSION"

# Detect which ctags we have
if echo "$CTAGS_VERSION" | grep -qi "universal\|exuberant"; then
    # Universal or Exuberant ctags
    echo "Detected Universal/Exuberant ctags"
    ctags -R \
        --exclude=build \
        --exclude=.git \
        --exclude=vendor \
        --exclude=third_party \
        --exclude=external_dependencies \
        --exclude=cmake-build-* \
        --exclude=out \
        --exclude=bin \
        --exclude=lib \
        libs/ apps/ tools/ tests/ 2>/dev/null || true
elif echo "$CTAGS_VERSION" | grep -qi "emacs"; then
    # GNU Emacs etags
    echo "Warning: GNU Emacs ctags detected. This creates TAGS format (for Emacs)."
    echo "For Vim/Neovim, you should install Universal Ctags:"
    echo "  Gentoo: sudo emerge dev-util/ctags"
    echo "  Or build from source: https://github.com/universal-ctags/ctags"
    echo ""
    echo "Attempting to generate TAGS file with etags..."
    find libs/ apps/ tools/ tests/ -type f \( -name "*.c" -o -name "*.h" \) \
        ! -path "*/build/*" \
        ! -path "*/.git/*" \
        ! -path "*/vendor/*" \
        ! -path "*/third_party/*" \
        ! -path "*/cmake-build-*/*" \
        -exec etags -a {} \; 2>/dev/null || true
else
    # Try generic approach
    echo "Unknown ctags version, attempting generic generation..."
    ctags -R libs/ apps/ tools/ tests/ 2>/dev/null || true
fi

# Check if tags file was created (tags for vim/ctags, TAGS for emacs/etags)
if [ -f "tags" ]; then
    TAG_COUNT=$(wc -l < tags)
    echo "Successfully generated tags file with $TAG_COUNT entries"
    echo "Tags file location: $SCRIPT_DIR/tags"
elif [ -f "TAGS" ]; then
    TAG_COUNT=$(wc -l < TAGS)
    echo "Successfully generated TAGS file (Emacs format) with $TAG_COUNT entries"
    echo "TAGS file location: $SCRIPT_DIR/TAGS"
    echo ""
    echo "Note: TAGS format is for Emacs. For Vim/Neovim, install Universal Ctags."
else
    echo "Warning: No tags file was created"
    exit 1
fi

echo "Done! You can now use tags in Vim/Neovim:"
echo "  - Ctrl+] to jump to definition"
echo "  - Ctrl+t to jump back"
echo "  - :tag <name> to search for a tag"
echo "  - :tselect <name> to select from multiple matches"
