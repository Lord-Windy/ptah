//
// Created by sam on 21/8/25.
//

#include "../include/samneural/activations.h"

#include <math.h>

float samneural_activation_leaky_relu(float x, float alpha) {
  return x > 0 ? x : alpha * x;
}

float samneural_activation_derivative_leaky_relu(float x, float alpha) {
  return x > 0 ? 1 : alpha;
}

void samneural_activation_softmax(uint64_t input_length, float *inputs, float *outputs) {

  float max_input = inputs[0];

  for (uint64_t i = 1; i < input_length; i++) {
    if (inputs[i] > max_input) {
      max_input = inputs[i];
    }
  }

  // Calculate exp(x - max) and sum
  float sum = 0;
  for (uint64_t i = 0; i < input_length; i++) {
    outputs[i] = expf(inputs[i] - max_input);
    sum += outputs[i];
  }

  // Normalize to get probabilities
  for (uint64_t i = 0; i < input_length; i++) {
    outputs[i] /= sum;
  }

}

void samneural_activation_derivative_softmax(uint64_t input_length, float *inputs, float *outputs) {

}