//
// Created by sam on 21/8/25.
//

#ifndef SAMNEURAL_ACTIVATIONS_H
#define SAMNEURAL_ACTIVATIONS_H

#include <stdint.h>

typedef enum { ACTIVATION_LEAKY_RELU, ACTIVATION_SOFTMAX } SamNeuralActivation;

float samneural_activation_leaky_relu(float x, float alpha);
void samneural_activation_softmax(uint64_t input_length, float *inputs, float *outputs);

float samneural_activation_derivative_leaky_relu(float x, float alpha);
void samneural_activation_derivative_softmax(uint64_t input_length, float *inputs, float *outputs);

#endif // SAMNEURAL_ACTIVATIONS_H
