// Copyright (c) Samuel "Lord Windy" Brown
// Licensed under the terms in LICENSE

const std = @import("std");

/// Calculate the wisdom score for a given input string.
/// Returns the sum of ASCII values of all characters.
pub fn calculateWisdom(input: []const u8) u32 {
    var wisdom: u32 = 0;
    for (input) |char| {
        wisdom += @as(u32, char);
    }
    return wisdom;
}

test "calculateWisdom with empty string" {
    const result = calculateWisdom("");
    try std.testing.expect(result == 0);
}

test "calculateWisdom with single character" {
    const result = calculateWisdom("A");
    try std.testing.expect(result == 65);
}

test "calculateWisdom with multiple characters" {
    const result = calculateWisdom("ABC");
    try std.testing.expect(result == 198); // 65 + 66 + 67
}

test "calculateWisdom with string" {
    const result = calculateWisdom("hello");
    try std.testing.expect(result == 532); // 104 + 101 + 108 + 108 + 111
}