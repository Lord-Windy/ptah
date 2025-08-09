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

// Core business logic (domain)
pub const Calculator = struct {
    value: i32,

    pub fn init(initial_value: i32) Calculator {
        return Calculator{ .value = initial_value };
    }

    pub fn add(self: *Calculator, number: i32) void {
        self.value += number;
    }

    pub fn multiply(self: *Calculator, number: i32) void {
        self.value *= number;
    }

    pub fn get_value(self: Calculator) i32 {
        return self.value;
    }
};

// Ports (interfaces for external interactions)
pub const InputPort = struct {
    add: fn (*Calculator, i32) void,
    multiply: fn (*Calculator, i32) void,
};

pub const OutputPort = struct {
    display_result: fn (i32) void,
};

// Adapters (concrete implementations of ports)
pub const ConsoleAdapter = struct {
    pub fn display_result(value: i32) void {
        std.debug.print("Result: {d}\n", .{value});
    }
};

// For testing
pub const TestAdapter = struct {
    pub fn display_result(value: i32) void {
        // Test adapter does nothing, just for demonstration
        _ = value;
    }
};

test "Calculator add functionality" {
    var calc = Calculator.init(5);
    calc.add(3);
    try std.testing.expect(calc.get_value() == 8);
}

test "Calculator multiply functionality" {
    var calc = Calculator.init(5);
    calc.multiply(3);
    try std.testing.expect(calc.get_value() == 15);
}

test "Calculator chain operations" {
    var calc = Calculator.init(2);
    calc.add(3);
    calc.multiply(4);
    try std.testing.expect(calc.get_value() == 20);
}
