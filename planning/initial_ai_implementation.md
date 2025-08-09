# AI Library Implementation Plan - Neural Networks with Backpropagation in Zig

## Overview

This plan outlines the development of a flexible AI library in Zig using
hexagonal architecture, starting with a neural network implementation using
backpropagation to solve MNIST digit classification.

## Architecture Overview

### Hexagonal Architecture Structure
```
┌─────────────────────────────────────────────────────┐
│                    Application                       │
│                   (MNIST App)                        │
└──────────────────┬──────────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────────┐
│                     Ports                           │
│  • Training Port    • Prediction Port               │
│  • Model Port       • Data Port                     │
└──────────────────┬──────────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────────┐
│                Domain Core                          │
│  • Network         • Layer                          │
│  • Neuron          • Activation Functions           │
│  • Loss Functions  • Optimizer                      │
└──────────────────┬──────────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────────┐
│                   Adapters                          │
│  • Neural Network Adapter                           │
│  • MNIST Data Adapter                               │
│  • Model Persistence Adapter                        │
└──────────────────────────────────────────────────────┘
```

## Phase 1: Core Domain Model (Week 1)

### 1.1 Define Core Interfaces
```zig
// src/domain/algorithm.zig
pub const Algorithm = struct {
    pub const TrainFn = fn(self: *Algorithm, inputs: []const []const f32, targets: []const []const f32) void;
    pub const PredictFn = fn(self: *const Algorithm, input: []const f32) []f32;
    
    train: TrainFn,
    predict: PredictFn,
};

// src/domain/layer.zig
pub const Layer = struct {
    neurons: u32,
    weights: [][]f32,
    biases: []f32,
    activation: ActivationFunction,
};

// src/domain/network.zig
pub const Network = struct {
    layers: []Layer,
    learning_rate: f32,
};
```

### 1.2 Activation Functions
- Implement ReLU, Sigmoid, Tanh, Softmax
- Each as a separate module with forward and derivative functions

### 1.3 Loss Functions
- Cross-entropy for classification
- Mean Squared Error for regression

## Phase 2: Backpropagation Implementation (Week 1-2)

### 2.1 Forward Propagation
```zig
// src/domain/neural_network/forward.zig
pub fn forward(network: *Network, input: []const f32) []f32 {
    // Layer-by-layer forward pass
    // Store intermediate activations for backprop
}
```

### 2.2 Backward Propagation
```zig
// src/domain/neural_network/backward.zig
pub fn backward(network: *Network, target: []const f32) void {
    // Calculate gradients using chain rule
    // Update weights and biases
}
```

### 2.3 Key Learning Concepts
- **Gradient Descent**: Minimize loss by moving weights in opposite direction of gradient
- **Chain Rule**: Compute gradients through composed functions
- **Learning Rate**: Controls step size in weight updates
- **Batch Processing**: Update weights after processing multiple examples

## Phase 3: Port Definitions (Week 2)

### 3.1 Training Port
```zig
// src/ports/training.zig
pub const TrainingPort = struct {
    pub const TrainFn = fn(data: DataSet, config: TrainingConfig) TrainingResult;
    pub const ValidateFn = fn(data: DataSet) ValidationResult;
};
```

### 3.2 Data Port
```zig
// src/ports/data.zig
pub const DataPort = struct {
    pub const LoadFn = fn(path: []const u8) DataSet;
    pub const BatchFn = fn(data: DataSet, batch_size: u32) Iterator;
};
```

### 3.3 Model Port
```zig
// src/ports/model.zig
pub const ModelPort = struct {
    pub const SaveFn = fn(network: Network, path: []const u8) void;
    pub const LoadFn = fn(path: []const u8) Network;
};
```

## Phase 4: MNIST Adapter Implementation (Week 2-3)

### 4.1 MNIST Data Loader
```zig
// src/adapters/mnist/loader.zig
pub fn loadMNIST(images_path: []const u8, labels_path: []const u8) DataSet {
    // Parse IDX file format
    // Normalize pixel values to [0, 1]
    // One-hot encode labels
}
```

### 4.2 Data Preprocessing
- Normalize pixel values: divide by 255
- Flatten 28x28 images to 784-dimensional vectors
- One-hot encode labels (10 classes)

## Phase 5: Neural Network Adapter (Week 3)

### 5.1 Network Builder
```zig
// src/adapters/neural_network/builder.zig
pub fn buildNetwork(config: NetworkConfig) Network {
    // Create layers with specified neurons
    // Initialize weights (Xavier/He initialization)
    // Set activation functions
}
```

### 5.2 Training Loop
```zig
// src/adapters/neural_network/trainer.zig
pub fn train(network: *Network, data: DataSet, epochs: u32, batch_size: u32) void {
    // Mini-batch gradient descent
    // Shuffle data each epoch
    // Track loss and accuracy
}
```

## Phase 6: MNIST Application (Week 3-4)

### 6.1 Application Structure
```zig
// src/app/mnist/main.zig
pub fn main() !void {
    // Load MNIST data
    // Create network: 784 -> 128 -> 64 -> 10
    // Train for 10 epochs
    // Evaluate on test set
    // Save trained model
}
```

### 6.2 Network Architecture for MNIST
- Input Layer: 784 neurons (28x28 pixels)
- Hidden Layer 1: 128 neurons, ReLU activation
- Hidden Layer 2: 64 neurons, ReLU activation
- Output Layer: 10 neurons, Softmax activation
- Loss: Cross-entropy
- Optimizer: Mini-batch SGD with learning rate 0.01

## Phase 7: Testing and Validation (Week 4)

### 7.1 Unit Tests
- Test activation functions and derivatives
- Test weight initialization
- Test forward/backward passes with known inputs

### 7.2 Integration Tests
- Test full training pipeline
- Verify loss decreases over epochs
- Check accuracy improves

### 7.3 MNIST Benchmarks
- Target: >95% accuracy on test set within 10 epochs
- Training time tracking
- Memory usage profiling

## Phase 8: Future Extensions (Post-MVP)

### 8.1 Algorithm Extensibility
- Design adapters for NEAT implementation
- Support for reinforcement learning algorithms
- Convolutional layer support

### 8.2 Optimizations
- Momentum and Adam optimizers
- Dropout for regularization
- Batch normalization

## Directory Structure
```
ptah/
├── src/
│   ├── domain/
│   │   ├── algorithm.zig
│   │   ├── network.zig
│   │   ├── layer.zig
│   │   ├── activation.zig
│   │   └── loss.zig
│   ├── ports/
│   │   ├── training.zig
│   │   ├── data.zig
│   │   └── model.zig
│   ├── adapters/
│   │   ├── neural_network/
│   │   │   ├── builder.zig
│   │   │   ├── trainer.zig
│   │   │   ├── forward.zig
│   │   │   └── backward.zig
│   │   └── mnist/
│   │       └── loader.zig
│   └── app/
│       └── mnist/
│           └── main.zig
├── test/
│   ├── unit/
│   └── integration/
├── data/
│   └── mnist/
└── build.zig
```

## Implementation Order

1. **Week 1**: Core domain models and basic neural network structure
2. **Week 1-2**: Implement backpropagation algorithm
3. **Week 2**: Define ports and create MNIST data adapter
4. **Week 3**: Complete neural network adapter and training loop
5. **Week 3-4**: Build MNIST application and achieve target accuracy
6. **Week 4**: Testing, optimization, and documentation

## Key Learning Resources

### Backpropagation Understanding
1. **Forward Pass**: Compute outputs layer by layer
2. **Loss Calculation**: Measure prediction error
3. **Backward Pass**: Compute gradients using chain rule
4. **Weight Update**: Adjust weights proportional to gradients

### Mathematical Foundation
- Matrix multiplication for layer computations
- Partial derivatives for gradient calculation
- Chain rule for multi-layer gradient flow

## Success Criteria

1. Clean hexagonal architecture with clear separation of concerns
2. Working MNIST classifier with >95% accuracy
3. Extensible design supporting future algorithms
4. Well-tested components with unit and integration tests
5. Clear understanding of backpropagation algorithm

## Notes for Implementation

- Start simple: implement a 2-layer network first
- Use f32 for all computations initially
- Print loss values during training to verify learning
- Save intermediate results for debugging
- Use small batches (32-64) for initial testing

This plan provides a structured approach to building the AI library while
learning the fundamentals of neural networks and backpropagation through
practical implementation.
