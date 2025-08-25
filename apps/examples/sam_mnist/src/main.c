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
#include <string.h>
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
  
  // Create neural network configuration
  printf("===================================\n");
  printf("Neural Network Configuration:\n");
  printf("===================================\n");
  
  SamNeuralConfiguration config = {
    .rng_seed = 42, //(uint64_t)time(NULL) + rand(),
    .thread_count = 1,
    .batch_size = 32,
    .epoch_count = 30,
    .learning_rate = 0.01f,  // Increased from 0.001
    .input_count = MNIST_IMAGE_SIZE,  // 784 pixels
    .output_count = MNIST_NUM_CLASSES, // 10 classes (0-9)
    .hidden_layer_count = 2,
    .hidden_layer_neuron_counts = NULL  // Will allocate below
  };
  
  // Allocate hidden layer configuration
  uint64_t* hidden_layers = samrena_push(arena, sizeof(uint64_t) * config.hidden_layer_count);
  if (!hidden_layers) {
    fprintf(stderr, "Failed to allocate hidden layer configuration\n");
    samrena_destroy(arena);
    return 1;
  }
  hidden_layers[0] = 128;  // First hidden layer: 128 neurons
  hidden_layers[1] = 64;   // Second hidden layer: 64 neurons
  config.hidden_layer_neuron_counts = hidden_layers;
  
  printf("  Input neurons: %lu\n", config.input_count);
  printf("  Hidden layers: %lu (", config.hidden_layer_count);
  for (uint64_t i = 0; i < config.hidden_layer_count; i++) {
    printf("%lu", config.hidden_layer_neuron_counts[i]);
    if (i < config.hidden_layer_count - 1) printf(", ");
  }
  printf(")\n");
  printf("  Output neurons: %lu\n", config.output_count);
  printf("  Learning rate: %.6f\n", config.learning_rate);
  printf("  Batch size: %lu\n", config.batch_size);
  printf("  Epochs: %lu\n", config.epoch_count);
  printf("\n");
  
  // Create neural network instance
  printf("Creating neural network...\n");
  SamNeuralInstance* neural_net = samneural_create(arena, config);
  if (!neural_net) {
    fprintf(stderr, "Failed to create neural network\n");
    samrena_destroy(arena);
    return 1;
  }
  printf("Neural network created successfully!\n\n");
  
  // Prepare training data in the format expected by samneural
  printf("===================================\n");
  printf("Training Phase:\n");
  printf("===================================\n");
  
  // Allocate buffers for training samples
  size_t samples_per_epoch = train_data->num_samples;
  float* training_inputs = samrena_push(arena, sizeof(float) * samples_per_epoch * MNIST_IMAGE_SIZE);
  float* training_targets = samrena_push(arena, sizeof(float) * samples_per_epoch * MNIST_NUM_CLASSES);
  
  if (!training_inputs || !training_targets) {
    fprintf(stderr, "Failed to allocate training buffers\n");
    samrena_destroy(arena);
    return 1;
  }
  
  // Convert MNIST data to neural network format
  for (size_t i = 0; i < samples_per_epoch; i++) {
    // Copy input pixels
    memcpy(&training_inputs[i * MNIST_IMAGE_SIZE], 
           train_data->samples[i].pixels, 
           sizeof(float) * MNIST_IMAGE_SIZE);
    
    // Convert label to one-hot encoding
    mnist_label_to_onehot(train_data->samples[i].label, 
                         &training_targets[i * MNIST_NUM_CLASSES]);
  }
  
  SamNeuralSamples training_samples = {
    .inputs = training_inputs,
    .target_outputs = training_targets,
    .sample_count = samples_per_epoch
  };
  
  // Train the network
  printf("Starting training with %zu samples...\n", samples_per_epoch);
  clock_t training_start = clock();
  
  samneural_train(neural_net, &training_samples);
  
  clock_t training_end = clock();
  double training_time = ((double)(training_end - training_start)) / CLOCKS_PER_SEC;
  printf("Training completed in %.2f seconds\n\n", training_time);
  
  // Test the network
  printf("===================================\n");
  printf("Testing Phase:\n");
  printf("===================================\n");
  
  // Prepare testing data
  size_t test_samples_count = test_data->num_samples;
  float* testing_inputs = samrena_push(arena, sizeof(float) * test_samples_count * MNIST_IMAGE_SIZE);
  float* testing_targets = samrena_push(arena, sizeof(float) * test_samples_count * MNIST_NUM_CLASSES);
  
  if (!testing_inputs || !testing_targets) {
    fprintf(stderr, "Failed to allocate testing buffers\n");
    samrena_destroy(arena);
    return 1;
  }
  
  // Convert test data to neural network format
  for (size_t i = 0; i < test_samples_count; i++) {
    memcpy(&testing_inputs[i * MNIST_IMAGE_SIZE], 
           test_data->samples[i].pixels, 
           sizeof(float) * MNIST_IMAGE_SIZE);
    
    mnist_label_to_onehot(test_data->samples[i].label, 
                         &testing_targets[i * MNIST_NUM_CLASSES]);
  }
  
  SamNeuralSamples testing_samples = {
    .inputs = testing_inputs,
    .target_outputs = testing_targets,
    .sample_count = test_samples_count
  };
  
  // Verify network performance
  printf("Testing network with %zu samples...\n", test_samples_count);
  clock_t testing_start = clock();
  
  uint64_t correct_predictions = samneural_verify(neural_net, &testing_samples);
  
  clock_t testing_end = clock();
  double testing_time = ((double)(testing_end - testing_start)) / CLOCKS_PER_SEC;
  
  double accuracy = ((double)correct_predictions / (double)test_samples_count) * 100.0;
  
  printf("Testing completed in %.2f seconds\n", testing_time);
  printf("Accuracy: %lu/%zu (%.2f%%)\n", correct_predictions, test_samples_count, accuracy);
  printf("\n");
  
  printf("===================================\n");
  printf("Training Summary:\n");
  printf("===================================\n");
  printf("Total training time: %.2f seconds\n", training_time);
  printf("Total testing time: %.2f seconds\n", testing_time);
  printf("Final accuracy: %.2f%%\n", accuracy);
  printf("===================================\n");
  
  // Clean up
  mnist_dataset_destroy(train_data);
  mnist_dataset_destroy(test_data);
  samrena_destroy(arena);
  
  return 0;
}