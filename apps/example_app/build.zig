const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Import the example library as a module
    const example_lib = b.addModule("example_lib", .{
        .root_source_file = b.path("../../libs/example_lib/src/main.zig"),
    });

    // Create the executable
    const exe = b.addExecutable(.{
        .name = "example_app",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    // Link with the example library
    exe.root_module.addImport("example_lib", example_lib);
    exe.linkLibC();

    // Install the executable
    b.installArtifact(exe);

    // Create a run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    // Create test executable for main_test.zig
    const tests = b.addTest(.{
        .root_source_file = b.path("src/main_test.zig"),
        .target = target,
        .optimize = optimize,
    });
    
    // Link the library to tests as well
    tests.root_module.addImport("example_lib", example_lib);

    // Add test step
    const test_step = b.step("test", "Run application tests");
    test_step.dependOn(&b.addRunArtifact(tests).step);
}
