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

#include <samneural.h>
#include <samrena.h>
#include "mnist_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
  // Check command line arguments
  if (argc != 3) {
    printf("Usage: %s <train.csv> <test.csv>\n", argv[0]);
    printf("  train.csv: CSV file with training data\n");
    printf("  test.csv:  CSV file with testing data\n");
    printf("\nExpected CSV format:\n");
    printf("  Header: label,1x1,1x2,...,28x28\n");
    printf("  Data: label (0-9),pixel values (0-255)\n");
    return 0;
  }
  
  const char* train_csv_path = argv[1];
  const char* test_csv_path = argv[2];
  
  printf("===================================\n");
  printf("MNIST Training Example\n");
  printf("Using samneural neural network library\n");
  printf("===================================\n\n");
  
  // Initialize random seed for shuffling
  srand(time(NULL));
  
  // Create arena for memory management
  Samrena* arena = samrena_create_default();
  if (!arena) {
    fprintf(stderr, "Failed to create arena\n");
    return 1;
  }
  
  // Load training data
  printf("Loading training data from: %s\n", train_csv_path);
  MnistDataset* train_data = mnist_dataset_create(arena);
  if (!train_data) {
    fprintf(stderr, "Failed to create training dataset\n");
    samrena_destroy(arena);
    return 1;
  }
  
  if (!mnist_load_csv(train_data, train_csv_path)) {
    fprintf(stderr, "Failed to load training CSV file\n");
    samrena_destroy(arena);
    return 1;
  }
  
  // Load testing data
  printf("Loading testing data from: %s\n", test_csv_path);
  MnistDataset* test_data = mnist_dataset_create(arena);
  if (!test_data) {
    fprintf(stderr, "Failed to create testing dataset\n");
    samrena_destroy(arena);
    return 1;
  }
  
  if (!mnist_load_csv(test_data, test_csv_path)) {
    fprintf(stderr, "Failed to load testing CSV file\n");
    samrena_destroy(arena);
    return 1;
  }
  
  printf("\n===================================\n");
  printf("Dataset Summary:\n");
  printf("  Training samples: %zu\n", train_data->num_samples);
  printf("  Testing samples:  %zu\n", test_data->num_samples);
  printf("===================================\n\n");
  
  // Show a sample from the training set for verification
  if (train_data->num_samples > 0) {
    printf("First training sample:\n");
    mnist_print_sample(&train_data->samples[0]);
    printf("\n");
  }
  
  // Clean up
  mnist_dataset_destroy(train_data);
  mnist_dataset_destroy(test_data);
  samrena_destroy(arena);
  
  return 0;
}