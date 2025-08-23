//
// Created by sam on 21/8/25.
//

#ifndef SAMNEURAL_NETWORK_H
#define SAMNEURAL_NETWORK_H

#include <samrena.h>
#include "activations.h"
#include "layers.h"

typedef struct {
  uint64_t layer_count;
  SamNeuralLayer **layers;

  uint64_t input_count;
  uint64_t output_count;

  // reusuable buffers
  float *gradient_buffer1;
  float *gradient_buffer2;

} SamNeuralNetwork;

SamNeuralNetwork *samneural_network_create(uint64_t hidden_layer_count,
                                           uint64_t *hidden_layer_neuron_counts,
                                           uint64_t input_count, uint64_t output_count,
                                           Samrena *samrena, SamRng *rng);

void samneural_network_activate(SamNeuralNetwork *network, const float *inputs);
void samneural_network_propagate_gradients(SamNeuralNetwork *network,
                                           const float *outputs_gradients);
void samneural_network_update_weights(SamNeuralNetwork *network, float learning_rate);
void samneural_network_zero_gradients(SamNeuralNetwork *network);
void samneural_network_get_outputs(SamNeuralNetwork *network, float *output);
#endif // SAMNEURAL_NETWORK_H
