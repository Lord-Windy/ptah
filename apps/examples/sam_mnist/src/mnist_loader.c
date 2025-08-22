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

#define _GNU_SOURCE  // For getline on Linux
#include "mnist_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

MnistDataset* mnist_dataset_create(Samrena* arena) {
    MnistDataset* dataset = (MnistDataset*)samrena_push(arena, sizeof(MnistDataset));
    if (!dataset) return NULL;
    
    dataset->samples = NULL;
    dataset->num_samples = 0;
    dataset->capacity = 0;
    dataset->arena = arena;
    
    return dataset;
}

static bool grow_dataset_capacity(MnistDataset* dataset, size_t min_capacity) {
    if (dataset->capacity >= min_capacity) return true;
    
    size_t new_capacity = dataset->capacity == 0 ? 1000 : dataset->capacity * 2;
    while (new_capacity < min_capacity) {
        new_capacity *= 2;
    }
    
    MnistSample* new_samples = (MnistSample*)samrena_push(
        dataset->arena, new_capacity * sizeof(MnistSample));
    if (!new_samples) return false;
    
    if (dataset->samples && dataset->num_samples > 0) {
        memcpy(new_samples, dataset->samples, dataset->num_samples * sizeof(MnistSample));
    }
    
    dataset->samples = new_samples;
    dataset->capacity = new_capacity;
    return true;
}

bool mnist_load_csv(MnistDataset* dataset, const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", path);
        return false;
    }
    
    char* line = NULL;
    size_t line_capacity = 0;
    ssize_t line_len;
    
    // Skip header line
    if (getline(&line, &line_capacity, file) == -1) {
        free(line);
        fclose(file);
        return false;
    }
    
    // Parse data lines
    while ((line_len = getline(&line, &line_capacity, file)) != -1) {
        if (line_len <= 1) continue;  // Skip empty lines
        
        // Ensure we have capacity
        if (dataset->num_samples >= dataset->capacity) {
            if (!grow_dataset_capacity(dataset, dataset->num_samples + 1)) {
                fprintf(stderr, "Error: Failed to allocate memory for samples\n");
                free(line);
                fclose(file);
                return false;
            }
        }
        
        MnistSample* sample = &dataset->samples[dataset->num_samples];
        
        // Parse the line
        char* token = strtok(line, ",");
        if (!token) continue;
        
        // First value is the label
        char* endptr;
        long label_val = strtol(token, &endptr, 10);
        if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r' && *endptr != ' ') {
            fprintf(stderr, "Warning: Invalid label found, skipping sample\n");
            continue;
        }
        
        if (label_val < 0 || label_val > 9) {
            fprintf(stderr, "Warning: Invalid label %ld found, skipping sample\n", label_val);
            continue;
        }
        sample->label = (uint8_t)label_val;
        
        // Next 784 values are pixels
        size_t pixel_idx = 0;
        while ((token = strtok(NULL, ",")) != NULL && pixel_idx < MNIST_IMAGE_SIZE) {
            // Trim whitespace
            while (*token == ' ' || *token == '\t') token++;
            
            long pixel_val = strtol(token, &endptr, 10);
            if (*endptr != '\0' && *endptr != '\n' && *endptr != '\r' && *endptr != ' ') {
                fprintf(stderr, "Warning: Invalid pixel value at index %zu for label %d, skipping sample\n", 
                        pixel_idx, sample->label);
                break;
            }
            
            if (pixel_val < 0) pixel_val = 0;
            if (pixel_val > 255) pixel_val = 255;
            
            // Normalize to [0, 1] range
            sample->pixels[pixel_idx] = (float)pixel_val / 255.0f;
            pixel_idx++;
        }
        
        // Only add sample if we got all 784 pixels
        if (pixel_idx == MNIST_IMAGE_SIZE) {
            dataset->num_samples++;
        } else {
            fprintf(stderr, "Warning: Sample with label %d has %zu pixels, expected 784\n", 
                    sample->label, pixel_idx);
        }
    }
    
    free(line);
    fclose(file);
    
    printf("Successfully loaded %zu samples from %s\n", dataset->num_samples, path);
    return dataset->num_samples > 0;
}

void mnist_get_batch(const MnistDataset* dataset, size_t start, size_t batch_size,
                     MnistSample** batch_out, size_t* actual_size) {
    if (!dataset || !batch_out || !actual_size) return;
    
    size_t end = start + batch_size;
    if (end > dataset->num_samples) {
        end = dataset->num_samples;
    }
    
    *batch_out = &dataset->samples[start];
    *actual_size = end - start;
}

void mnist_shuffle(MnistDataset* dataset) {
    if (!dataset || dataset->num_samples <= 1) return;
    
    // Fisher-Yates shuffle
    for (size_t i = dataset->num_samples - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        
        // Swap samples[i] and samples[j]
        MnistSample temp = dataset->samples[i];
        dataset->samples[i] = dataset->samples[j];
        dataset->samples[j] = temp;
    }
}

void mnist_label_to_onehot(uint8_t label, float* onehot_out) {
    if (!onehot_out) return;
    
    for (int i = 0; i < MNIST_NUM_CLASSES; i++) {
        onehot_out[i] = 0.0f;
    }
    
    if (label < MNIST_NUM_CLASSES) {
        onehot_out[label] = 1.0f;
    }
}

void mnist_print_sample(const MnistSample* sample) {
    if (!sample) return;
    
    printf("Label: %d\n", sample->label);
    for (size_t y = 0; y < 28; y++) {
        for (size_t x = 0; x < 28; x++) {
            float pixel = sample->pixels[y * 28 + x];
            if (pixel > 0.75f) {
                printf("█");
            } else if (pixel > 0.5f) {
                printf("▓");
            } else if (pixel > 0.25f) {
                printf("▒");
            } else if (pixel > 0.1f) {
                printf("░");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}

void mnist_dataset_destroy(MnistDataset* dataset) {
    // Since we're using arena allocation, we don't need to free individual allocations
    // The arena will handle all cleanup
    if (dataset) {
        dataset->samples = NULL;
        dataset->num_samples = 0;
        dataset->capacity = 0;
    }
}