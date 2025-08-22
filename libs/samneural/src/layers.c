//
// Created by sam on 21/8/25.
//

#include "../include/samneural/layers.h"

#include <math.h>
#include <string.h>

SamNeuralLayer *samneural_layer_create(uint64_t neuron_count, uint64_t input_count,
                                       SamNeuralActivation activation, Samrena *samrena,
                                       SamRng *rng) {
  SamNeuralLayer *layer = SAMRENA_PUSH_TYPE(samrena, SamNeuralLayer);

  layer->neuron_count = neuron_count;
  layer->input_count = input_count;
  layer->activation = activation;

  layer->weights = SAMRENA_PUSH_ARRAY(samrena, float, neuron_count *input_count);
  layer->biases = SAMRENA_PUSH_ARRAY(samrena, float, neuron_count);
  layer->activations = SAMRENA_PUSH_ARRAY(samrena, float, neuron_count);

  layer->weights_gradients = SAMRENA_PUSH_ARRAY(samrena, float, neuron_count *input_count);
  layer->biases_gradients = SAMRENA_PUSH_ARRAY(samrena, float, neuron_count);
  layer->activations_gradients = SAMRENA_PUSH_ARRAY(samrena, float, neuron_count);

  layer->raw_outputs = SAMRENA_PUSH_ARRAY(samrena, float, neuron_count);
  layer->last_inputs = SAMRENA_PUSH_ARRAY(samrena, float, input_count);

  // initialize biases and gradients
  for (int i = 0; i < neuron_count; i++) {
    layer->biases[i] = (samrng_float(rng) * 2.0f - 1.0f) * 0.01f; // small initial value
    layer->biases_gradients[i] = 0.0f;
    layer->activations_gradients[i] = 0.0f;
  }

  // initialize weights
  const double din = (double)input_count;
  const float scale = (float)(1.0 / sqrt(din));
  for (int i = 0; i < neuron_count * input_count; i++) {
    layer->weights[i] = (samrng_float(rng) * 2.0f - 1.0f) * scale; // small initial value
    layer->weights_gradients[i] = 0.0f;
  }

  return layer;
}

void samneural_layer_activate(SamNeuralLayer *layer, const float *inputs) {

  if (layer == NULL || inputs == NULL) {
    return;
  }

  // store inputs for backwards pass
  memcpy(layer->last_inputs, inputs, sizeof(float) * layer->input_count);

  for (int i = 0; i < layer->neuron_count; i++) {
    float sum = 0.0f;
    for (int j = 0; j < layer->input_count; j++) {
      sum += layer->weights[i * layer->input_count + j] * inputs[j];
    }
    sum += layer->biases[i];
    layer->raw_outputs[i] = sum;

    switch (layer->activation) {
      case ACTIVATION_LEAKY_RELU:
        layer->activations[i] = samneural_activation_leaky_relu(sum, 0.01f);
        break;
      case ACTIVATION_SOFTMAX:
        samneural_activation_softmax(layer->neuron_count, layer->raw_outputs, layer->activations);
        break;
    }
  }
}

void samneural_layer_propagate_gradients(SamNeuralLayer *layer, float *input_gradients,
                                         const float *outputs_gradients) {

  if (layer == NULL || input_gradients == NULL || outputs_gradients == NULL) {
    return;
  }

  float dot_product = 0.0f;
  float derivative = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float ag = 0.0f;
  float *w_row = NULL;
  float *wg_row = NULL;

  switch (layer->activation) {
    case ACTIVATION_LEAKY_RELU:
      // Fused pass:
      // - compute activation gradient per neuron
      // - accumulate bias gradient
      // - accumulate weight gradients
      // - accumulate input gradients

      for (int i = 0; i < layer->neuron_count; i++) {
        z = layer->raw_outputs[i];
        float derive = z > 0.0f ? 1.0f : 0.01f;
        ag = outputs_gradients[i] * derive;

        layer->biases_gradients[i] += ag;

        w_row = &layer->weights[i * layer->input_count];
        wg_row = &layer->weights_gradients[i * layer->input_count];

        for (int j = 0; j < layer->input_count; j++) {
          wg_row[j] = ag * layer->last_inputs[j];
          input_gradients[j] += w_row[j] * ag;
        }
      }
      break;
    case ACTIVATION_SOFTMAX:
      // Softmax Jacobian-vector product:
      // ag[i] = y[i] * (grad_output[i] - sum_k y[k]*grad_output[k])

      for (int i = 0; i < layer->neuron_count; i++) {
        dot_product += layer->activations[i] * outputs_gradients[i];
      }

      for (int i = 0; i < layer->neuron_count; i++) {
        y = layer->activations[i];
        ag = y * (outputs_gradients[i] - dot_product);

        layer->biases_gradients[i] += ag;

        w_row = &layer->weights[i * layer->input_count];
        wg_row = &layer->weights_gradients[i * layer->input_count];

        for (int j = 0; j < layer->input_count; j++) {
          wg_row[j] = ag * layer->last_inputs[j];
          input_gradients[j] += w_row[j] * ag;
        }
      }

      break;
  }
}

void samneural_layer_update_weights(SamNeuralLayer *layer, float learning_rate) {
  for (int i = 0; i < layer->neuron_count * layer->input_count; i++) {
    layer->weights[i] -= learning_rate * layer->weights_gradients[i];
  }

  for (int i = 0; i < layer->neuron_count; i++) {
    layer->biases[i] -= learning_rate * layer->biases_gradients[i];
  }
}

void samneural_layer_zero_gradients(SamNeuralLayer *layer) {
  if (layer == NULL) {
    return;
  }

  memset(layer->weights_gradients, 0, sizeof(float) * layer->neuron_count * layer->input_count);
  memset(layer->biases_gradients, 0, sizeof(float) * layer->neuron_count);
  memset(layer->activations_gradients, 0, sizeof(float) * layer->neuron_count);
}