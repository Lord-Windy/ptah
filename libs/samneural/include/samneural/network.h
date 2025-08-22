//
// Created by sam on 21/8/25.
//

#ifndef SAMNEURAL_NETWORK_H
#define SAMNEURAL_NETWORK_H

#include "layers.h"

typedef struct {
  uint64_t layer_count;
  SamNeuralLayer *layers;

  uint64_t input_count;
  uint64_t output_count;

  // reusuable buffers
  float *gradient_buffer1;
  float *gradient_buffer2;

} SamNeuralNetwork;

SamNeuralNetwork *samneural_network_create(uint64_t hidden_layer_count, uint64_t *layer_neuron_counts,
                                           uint64_t input_count, uint64_t output_count,
                                           Samrena *samrena, SamRng *rng);

#endif // SAMNEURAL_NETWORK_H
