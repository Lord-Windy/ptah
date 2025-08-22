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

# Run MNIST example with hardcoded dataset paths
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

TRAIN_CSV="/home/sam/Dropbox/ai_training_data/mnist/train.csv"
TEST_CSV="/home/sam/Dropbox/ai_training_data/mnist/test.csv"

# Check if the binary exists
if [ ! -f "$PROJECT_ROOT/build/bin/sam_mnist" ]; then
    echo "Error: sam_mnist binary not found. Please build the project first."
    echo "Run: cd build && make sam_mnist"
    exit 1
fi

# Check if CSV files exist
if [ ! -f "$TRAIN_CSV" ]; then
    echo "Error: Training CSV not found at $TRAIN_CSV"
    exit 1
fi

if [ ! -f "$TEST_CSV" ]; then
    echo "Error: Test CSV not found at $TEST_CSV"
    exit 1
fi

# Run the MNIST example
echo "Running MNIST example with:"
echo "  Training data: $TRAIN_CSV"
echo "  Test data: $TEST_CSV"
echo ""

"$PROJECT_ROOT/build/bin/sam_mnist" "$TRAIN_CSV" "$TEST_CSV"