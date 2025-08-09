// Copyright 2025 Samuel "Lord Windy" Brown
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

const std = @import("std");
const example_lib = @import("example_lib");

// Import the library components
const Calculator = example_lib.Calculator;
const ConsoleAdapter = example_lib.ConsoleAdapter;

pub fn main() !void {
    // Create a calculator instance (core domain)
    var calc = Calculator.init(0);
    
    // Simulate some business logic operations
    calc.add(10);
    calc.multiply(2);
    
    // Use adapter to display result (infrastructure layer)
    ConsoleAdapter.display_result(calc.get_value());
    
    // More operations
    calc.add(5);
    ConsoleAdapter.display_result(calc.get_value());
}

// Application-specific logic that uses the library
pub const Application = struct {
    allocator: std.mem.Allocator,
    calculator: Calculator,
    
    pub fn init(allocator: std.mem.Allocator, initial_value: i32) Application {
        return Application{
            .allocator = allocator,
            .calculator = Calculator.init(initial_value),
        };
    }
    
    pub fn run(self: *Application) void {
        self.calculator.add(10);
        self.calculator.multiply(2);
        ConsoleAdapter.display_result(self.calculator.get_value());
    }
};
