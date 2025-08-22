//
// Created by sam on 22/8/25.
//

#ifndef PTAH_LOSS_H
#define PTAH_LOSS_H

#include <stdint.h>

float samneural_loss_cross_entropy(float *predictions, float *targets, uint64_t count);
void samneural_loss_cross_entropy_derivative(float *predictions, float *targets, float* gradients, uint64_t count);

#endif // PTAH_LOSS_H
