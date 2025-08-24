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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

SamNeuralInstance *samneural_create(Samrena* samrena, SamNeuralConfiguration config) {
  SamNeuralInstance *instance = SAMRENA_PUSH_TYPE(samrena, SamNeuralInstance);
  instance->rng = samrng_create(samrena, config.rng_seed);
  instance->output_buffer = SAMRENA_PUSH_ARRAY(samrena, float, config.output_count);
  instance->gradient_buffer = SAMRENA_PUSH_ARRAY(samrena, float, config.output_count);

  instance->network = samneural_network_create(config.hidden_layer_count,
    config.hidden_layer_neuron_counts, config.input_count, config.output_count, samrena,
    instance->rng);

  memcpy(&instance->configuration, &config, sizeof(SamNeuralConfiguration));

  return instance;
}

uint64_t max_position(float *array, uint64_t count) {

  float max = array[0];
  uint64_t max_position = 0;

  for (uint64_t i = 1; i < count; i++) {
    if (array[i] > max) {
      max = array[i];
      max_position = i;
    }
  }

  return max_position;
}

static void shuffle_indices(uint64_t *indices, uint64_t count, SamRng *rng) {
  // Initialize indices
  for (uint64_t i = 0; i < count; i++) {
    indices[i] = i;
  }
  
  // Fisher-Yates shuffle
  for (uint64_t i = count - 1; i > 0; i--) {
    uint64_t j = samrng_uint64(rng) % (i + 1);
    uint64_t temp = indices[i];
    indices[i] = indices[j];
    indices[j] = temp;
  }
}

void samneural_train(SamNeuralInstance *instance, SamNeuralSamples *samples) {
  
  // Allocate indices array for shuffling
  uint64_t *indices = malloc(sizeof(uint64_t) * samples->sample_count);
  if (!indices) {
    printf("Failed to allocate memory for shuffling\n");
    return;
  }

  for (uint64_t epoch = 0; epoch < instance->configuration.epoch_count; epoch++) {
    clock_t epoch_start = clock();
    
    // Shuffle sample order for this epoch
    shuffle_indices(indices, samples->sample_count, instance->rng);
    float epoch_loss = 0.0f;
    uint64_t correct_predictions = 0;
    uint64_t batch_count = 0;
    uint64_t num_batch = 0;

    samneural_network_zero_gradients(instance->network);

    for (int idx = 0; idx < samples->sample_count; idx++) {
      int i = indices[idx];  // Use shuffled index
      
      uint64_t input_position = i * instance->network->input_count;
      uint64_t output_position = i * instance->network->output_count;
      samneural_network_activate(instance->network, &samples->inputs[input_position]);
      samneural_network_get_outputs(instance->network, instance->output_buffer);

      float *target_outputs = &samples->target_outputs[output_position];

      float loss = samneural_loss_cross_entropy(instance->output_buffer, target_outputs, instance->network->output_count);
      epoch_loss += loss;
      uint64_t target = max_position(target_outputs, instance->network->output_count);
      uint64_t prediction = max_position(instance->output_buffer, instance->network->output_count);
      correct_predictions += prediction == target;

      samneural_loss_cross_entropy_derivative(instance->output_buffer, target_outputs, instance->gradient_buffer, instance->network->output_count);
      
      samneural_network_propagate_gradients(instance->network, instance->gradient_buffer);

      batch_count++;

      const bool batch_full = (batch_count == instance->configuration.batch_size);
      const bool last_sample = (idx == samples->sample_count - 1);
      if (batch_full || last_sample) {

        // Scale gradients by actual batch size (important for last batch which might be smaller)
        float scale = 1.0f / (float)batch_count;
        
        for (uint64_t j = 0; j < instance->network->layer_count; j++) {
          SamNeuralLayer *layer = instance->network->layers[j];
          
          for (uint64_t k = 0; k < layer->neuron_count; k++) {
            layer->biases_gradients[k] *= scale;
          }
          
          uint64_t wcount = layer->neuron_count * layer->input_count;
          for (uint64_t k = 0; k < wcount; k++) {
            layer->weights_gradients[k] *= scale;
          }
        }

        samneural_network_update_weights(instance->network, instance->configuration.learning_rate);
        samneural_network_zero_gradients(instance->network);

        batch_count = 0;
        num_batch++;
      }

    }

    clock_t epoch_end = clock();
    double epoch_time = ((double)(epoch_end - epoch_start)) / CLOCKS_PER_SEC;
    double accuracy = (double)correct_predictions / (double)samples->sample_count * 100.0;
    
    printf("Epoch %lu: Time: %.3fs, Accuracy: %.2f%% (%lu/%d correct), Loss: %.6f\n",
           epoch + 1, epoch_time, accuracy, correct_predictions, samples->sample_count, epoch_loss);

  }
  
  free(indices);
}

// Returns correct number of samples
uint64_t samneural_verify(SamNeuralInstance *instance, SamNeuralSamples *samples) {
  uint64_t correct_predictions = 0;
  
  for (uint64_t i = 0; i < samples->sample_count; i++) {
    uint64_t input_position = i * instance->network->input_count;
    uint64_t output_position = i * instance->network->output_count;
    
    samneural_network_activate(instance->network, &samples->inputs[input_position]);
    samneural_network_get_outputs(instance->network, instance->output_buffer);
    
    float *target_outputs = &samples->target_outputs[output_position];
    
    uint64_t target = max_position(target_outputs, instance->network->output_count);
    uint64_t prediction = max_position(instance->output_buffer, instance->network->output_count);
    
    if (prediction == target) {
      correct_predictions++;
    }
  }
  
  return correct_predictions;
}

void samneural_hello(void) { printf("Hello from samneural!\n"); }