//
// Created by sam on 22/8/25.
//

#include "../include/samneural/loss.h"

#include <math.h>

float samneural_loss_cross_entropy(float *predictions, float *targets, uint64_t count) {

  float loss = 0.0f;

  for (uint64_t i = 0; i < count; i++) {
    float target = targets[i];
    float prediction = predictions[i];

    float epsilon = 1e-15f;
    float safe_prediction = prediction > epsilon ? prediction : epsilon;

    loss -= target * logf(safe_prediction);
  }

  return loss;
}

void samneural_loss_cross_entropy_derivative(float *predictions, float *targets, float* gradients, uint64_t count) {

  for (uint64_t i = 0; i < count; i++) {
    gradients[i] = predictions[i] - targets[i];
  }

}