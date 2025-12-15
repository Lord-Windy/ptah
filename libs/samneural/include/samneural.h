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

#ifndef SAMNEURAL_H
#define SAMNEURAL_H

#include "samneural/activations.h"
#include "samneural/layers.h"
#include "samneural/loss.h"
#include "samneural/network.h"
#include <samdata.h>
#include <samrena.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Data item to be processed with targed outputs
 */
typedef struct {
  float *inputs;
  float *target_outputs;
  uint64_t sample_count;
} SamNeuralSamples;

typedef struct {
  uint64_t rng_seed;
  uint64_t thread_count;
  uint64_t batch_size;
  uint64_t epoch_count;
  float learning_rate;
  uint64_t input_count;
  uint64_t output_count;
  uint64_t hidden_layer_count;
  uint64_t *hidden_layer_neuron_counts;
} SamNeuralConfiguration;

typedef struct {
  SamNeuralNetwork *network;
  SamNeuralConfiguration configuration;
  SamRng *rng;
  float *output_buffer;
  float *gradient_buffer;
} SamNeuralInstance;

SamNeuralInstance *samneural_create(Samrena *samrena, SamNeuralConfiguration config);
void samneural_train(SamNeuralInstance *instance, SamNeuralSamples *samples);

// Returns number of successes
uint64_t samneural_verify(SamNeuralInstance *instance, SamNeuralSamples *samples);

void samneural_hello(void);

#ifdef __cplusplus
}
#endif

#endif