#!/bin/bash
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

# Compile all .dot files in docs/dot/src to the specified output format

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_ROOT/docs/dot/src"
OUT_DIR="$PROJECT_ROOT/docs/dot"

# Default format
FORMAT="${1:-svg}"

# Validate format
case "$FORMAT" in
    svg|png|pdf|ps|eps|jpg|jpeg|gif|bmp|webp)
        ;;
    *)
        echo "Usage: $0 [format]"
        echo ""
        echo "Supported formats: svg, png, pdf, ps, eps, jpg, jpeg, gif, bmp, webp"
        echo "Default: svg"
        exit 1
        ;;
esac

# Check if dot is installed
if ! command -v dot &> /dev/null; then
    echo "Error: graphviz 'dot' command not found. Please install graphviz."
    exit 1
fi

# Check if source directory exists
if [ ! -d "$SRC_DIR" ]; then
    echo "Error: Source directory $SRC_DIR does not exist."
    exit 1
fi

# Count files
count=0

# Compile each .dot file
for dotfile in "$SRC_DIR"/*.dot; do
    if [ -f "$dotfile" ]; then
        basename=$(basename "$dotfile" .dot)
        outfile="$OUT_DIR/$basename.$FORMAT"
        echo "Compiling: $dotfile -> $outfile"
        dot -T"$FORMAT" "$dotfile" -o "$outfile"
        count=$((count + 1))
    fi
done

if [ $count -eq 0 ]; then
    echo "No .dot files found in $SRC_DIR"
else
    echo "Compiled $count file(s) to $FORMAT format."
fi
