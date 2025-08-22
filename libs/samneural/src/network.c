//
// Created by sam on 21/8/25.
//

#include "../include/samneural/network.h"

#include <string.h>

SamNeuralNetwork *samneural_network_create(uint64_t hidden_layer_count,
                                           uint64_t *hidden_layer_neuron_counts,
                                           uint64_t input_count, uint64_t output_count,
                                           Samrena *samrena, SamRng *rng) {

  SamNeuralNetwork *network = SAMRENA_PUSH_TYPE(samrena, SamNeuralNetwork);

  network->input_count = input_count;
  network->output_count = output_count;

  network->layers = SAMRENA_PUSH_ARRAY(samrena, SamNeuralLayer*, hidden_layer_count + 1); //+1 for output
  network->layer_count = hidden_layer_count + 1;

  // hidden layers
  for (int i = 0; i < hidden_layer_count; i++) {
    if (i == 0) {
      network->layers[i] = samneural_layer_create(hidden_layer_neuron_counts[i], input_count,
                                                ACTIVATION_LEAKY_RELU, samrena, rng);
    } else {
      network->layers[i] = samneural_layer_create(hidden_layer_neuron_counts[i], network->layers[i - 1]->neuron_count,
                                                ACTIVATION_LEAKY_RELU, samrena, rng);
    }
  }

  //output layer
  SamNeuralActivation output_activation = output_count > 0 ? ACTIVATION_SOFTMAX : ACTIVATION_LEAKY_RELU;
  if (hidden_layer_count > 0) {
    network->layers[hidden_layer_count] = samneural_layer_create(output_count, network->layers[hidden_layer_count - 1]->neuron_count,
                                                output_activation, samrena, rng);
  } else {
    // No hidden layers, inputs directly to output
    network->layers[hidden_layer_count] = samneural_layer_create(output_count, input_count,
                                                output_activation, samrena, rng);
  }

  uint64_t max_neuron_count = 0;
  for (int i = 0; i < network->layer_count; i++) {
    if (network->layers[i]->neuron_count > max_neuron_count) {
      max_neuron_count = network->layers[i]->neuron_count;
    }
  }

  network->gradient_buffer1 = SAMRENA_PUSH_ARRAY(samrena, float, max_neuron_count);
  network->gradient_buffer2 = SAMRENA_PUSH_ARRAY(samrena, float, max_neuron_count);

  return network;
}

void samneural_network_activate(SamNeuralNetwork *network, const float *inputs) {
  samneural_layer_activate(network->layers[0], inputs);
  for (int i = 1; i < network->layer_count; i++) {
    samneural_layer_activate(network->layers[i], network->layers[i - 1]->raw_outputs);
  }
}

void samneural_network_propagate_gradients(SamNeuralNetwork *network, float *input_gradients,
                                           const float *outputs_gradients) {

}


void samneural_network_update_weights(SamNeuralNetwork *network, float learning_rate) {
  for (int i = 0; i < network->layer_count; i++) {
    samneural_layer_update_weights(network->layers[i], learning_rate);
  }
}

void samneural_network_zero_gradients(SamNeuralNetwork *network) {
  for (int i = 0; i < network->layer_count; i++) {
    samneural_layer_zero_gradients(network->layers[i]);
  }
}

void sameural_get_output(SamNeuralNetwork *network, float *output) {
  // memcopy the final layers outputs to outputs
  memcpy(output, network->layers[network->layer_count - 1]->raw_outputs, network->output_count * sizeof(float));
}