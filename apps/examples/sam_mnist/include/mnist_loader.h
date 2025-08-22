/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
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

#ifndef MNIST_LOADER_H
#define MNIST_LOADER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "samrena.h"

#define MNIST_IMAGE_SIZE 784  // 28x28 pixels
#define MNIST_NUM_CLASSES 10

typedef struct {
    uint8_t label;
    float pixels[MNIST_IMAGE_SIZE];  // Normalized to [0, 1]
} MnistSample;

typedef struct {
    MnistSample* samples;
    size_t num_samples;
    size_t capacity;
    Samrena* arena;
} MnistDataset;

// Initialize dataset
MnistDataset* mnist_dataset_create(Samrena* arena);

// Load CSV file
bool mnist_load_csv(MnistDataset* dataset, const char* path);

// Get a batch of samples
void mnist_get_batch(const MnistDataset* dataset, size_t start, size_t batch_size, 
                     MnistSample** batch_out, size_t* actual_size);

// Shuffle the dataset (for training)
void mnist_shuffle(MnistDataset* dataset);

// Convert label to one-hot encoding
void mnist_label_to_onehot(uint8_t label, float* onehot_out);

// Print sample as ASCII art (for debugging)
void mnist_print_sample(const MnistSample* sample);

// Clean up
void mnist_dataset_destroy(MnistDataset* dataset);

#endif // MNIST_LOADER_H