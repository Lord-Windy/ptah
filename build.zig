const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Add the example library as a module
    const example_lib = b.addModule("example_lib", .{
        .root_source_file = b.path("libs/example_lib/src/main.zig"),
    });

    // Add the thoth library as a module
    _ = b.addModule("thoth", .{
        .root_source_file = b.path("libs/thoth/src/root.zig"),
    });

    // Add the example app
    const example_app_exe = b.addExecutable(.{
        .name = "example_app",
        .root_source_file = b.path("apps/example_app/src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    
    // Link the library module to the app
    example_app_exe.root_module.addImport("example_lib", example_lib);

    // Install the app executable
    b.installArtifact(example_app_exe);

    // Create a run step
    const run_cmd = b.addRunArtifact(example_app_exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the example app");
    run_step.dependOn(&run_cmd.step);

    // Build the library tests
    const lib_tests = b.addTest(.{
        .root_source_file = b.path("libs/example_lib/src/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    const lib_separate_tests = b.addTest(.{
        .root_source_file = b.path("libs/example_lib/src/main_test.zig"),
        .target = target,
        .optimize = optimize,
    });

    const thoth_tests = b.addTest(.{
        .root_source_file = b.path("libs/thoth/src/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    // Build the app tests
    const app_tests = b.addTest(.{
        .root_source_file = b.path("apps/example_app/src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    app_tests.root_module.addImport("example_lib", example_lib);

    const app_separate_tests = b.addTest(.{
        .root_source_file = b.path("apps/example_app/src/main_test.zig"),
        .target = target,
        .optimize = optimize,
    });
    app_separate_tests.root_module.addImport("example_lib", example_lib);

    // Create test step
    const test_step = b.step("test", "Run all tests");
    test_step.dependOn(&lib_tests.step);
    test_step.dependOn(&lib_separate_tests.step);
    test_step.dependOn(&thoth_tests.step);
    test_step.dependOn(&app_tests.step);
    test_step.dependOn(&app_separate_tests.step);
}
