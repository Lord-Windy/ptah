
#ifndef SAMNEURAL_LAYER_H
#define SAMNEURAL_LAYER_H

#include <samdata.h>
#include <samrena.h>
#include <stdint.h>
#include "activations.h"

typedef struct {

  uint64_t neuron_count; //outputs as well since I am an idiot who keeps forgetting
  uint64_t input_count;
  float *activations;
  float *weights; // neuron * inputs
  float *biases;
  SamNeuralActivation activation;

  //back propagation stuff from my old instance, but honestly this was done by ai so I will be
  // implementing this all by hand to be sure

  float *weights_gradients;
  float *biases_gradients;
  float *activations_gradients;

  // Store raw outputs (pre-activation) for derivative computation
  float *raw_outputs;
  // Store inputs from forward pass for gradient computation
  float *last_inputs;


} SamNeuralLayer;

SamNeuralLayer* samneural_layer_create(uint64_t neuron_count, uint64_t input_count, SamNeuralActivation activation, Samrena *samrena, SamRng *rng);
void samneural_layer_activate(SamNeuralLayer *layer, const float *inputs);
void samneural_layer_propagate_gradients(SamNeuralLayer *layer, float *input_gradients, const float *outputs_gradients);
void samneural_layer_update_weights(SamNeuralLayer *layer, float learning_rate);
void samneural_layer_zero_gradients(SamNeuralLayer *layer);

#endif // SAMNEURAL_LAYER_H