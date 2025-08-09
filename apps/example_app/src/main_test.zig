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
const TestAdapter = example_lib.TestAdapter;

const Application = @import("main.zig").Application;

test "Application initialization" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    
    const app = Application.init(gpa.allocator(), 0);
    try std.testing.expect(app.calculator.get_value() == 0);
}

test "Application run functionality" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    
    var app = Application.init(gpa.allocator(), 5);
    app.run(); // This adds 10 and multiplies by 2, so 5+10 = 15, 15*2 = 30
    
    // We can't easily test the output since it goes to console,
    // but we can verify the calculator state
    try std.testing.expect(app.calculator.get_value() == 30);
}

test "Calculator integration in application" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    
    var app = Application.init(gpa.allocator(), 1);
    app.calculator.add(5);
    app.calculator.multiply(2);
    
    try std.testing.expect(app.calculator.get_value() == 12);
}
