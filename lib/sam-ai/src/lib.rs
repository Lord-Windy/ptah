// Copyright 2025 Samuel "Lord Windy" Brown
// Licensed under the Apache License, Version 2.0
//
// A minimal wgpu compute example exposed as a library function.
// It doubles an array of u32 values on the GPU and returns the result.

use anyhow::Context;


pub async fn run_compute_example_async(input: &[u32]) -> anyhow::Result<Vec<u32>> {
    // 1. Instance/Adapter/Device
    let instance = wgpu::Instance::default();

    // let adapter = instance
    //     .request_adapter(&wgpu::RequestAdapterOptions {
    //         power_preference: wgpu::PowerPreference::HighPerformance,
    //         force_fallback_adapter: false,
    //         compatible_surface: None,
    //     })
    //     .await
    //     .map_err(|e| anyhow::anyhow!("No suitable GPU adapters found on the system: {e}"))?;

    let adapter = pollster::block_on(instance.request_adapter(&wgpu::RequestAdapterOptions::default()))
        .context("No suitable GPU adapters found on the system")?;

    let downlevel_capabilities = adapter.get_downlevel_capabilities();
    if !downlevel_capabilities
        .flags
        .contains(wgpu::DownlevelFlags::COMPUTE_SHADERS)
    {
        panic!("Adapter does not support compute shaders");
    }

    let (device, queue) = pollster::block_on(adapter
        .request_device(
            &wgpu::DeviceDescriptor {
                label: Some("sam-ai-device"),
                required_features: wgpu::Features::empty(),
                required_limits: wgpu::Limits::downlevel_defaults(),
                memory_hints: wgpu::MemoryHints::default(),
                trace: wgpu::Trace::Off,
            },
        )).context("Failed to create device")?;

    let shader_src = include_str!("shaders/double.wgsl");
    let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: Some("Compute Shader - Double"),
        source: wgpu::ShaderSource::Wgsl(shader_src.into()),
    });

    // 2. Create buffers
    let size_bytes = (input.len() * std::mem::size_of::<u32>()) as wgpu::BufferAddress;

    let input_data_buffer = device.create_buffer_init(&wgpu::util::BufferInitDescriptor {
        label: Some("Input Buffer"),
        contents: bytemuck::cast_slice(input),
        usage: wgpu::BufferUsages::STORAGE,
    });

    let output_data_buffer = device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("Output Buffer"),
        size: input_data_buffer.size(),
        usage: wgpu::BufferUsages::STORAGE | wgpu::BufferUsages::COPY_SRC,
        mapped_at_creation: false,
    });

    let download_buffer = device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("Download Buffer"),
        size: input_data_buffer.size(),
        usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
        mapped_at_creation: false,
    });


    let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("BGL"),
        entries: &[
            wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::COMPUTE,
                ty: wgpu::BindingType::Buffer {
                    ty: wgpu::BufferBindingType::Storage { read_only: true },
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 1,
                visibility: wgpu::ShaderStages::COMPUTE,
                ty: wgpu::BindingType::Buffer {
                    ty: wgpu::BufferBindingType::Storage { read_only: false },
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            },
        ],
    });

    let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("Bind Group"),
        layout: &bind_group_layout,
        entries: &[
            wgpu::BindGroupEntry { binding: 0, resource: input_data_buffer.as_entire_binding() },
            wgpu::BindGroupEntry { binding: 1, resource: output_data_buffer.as_entire_binding() },
        ],
    });

    let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: Some("Pipeline Layout"),
        bind_group_layouts: &[&bind_group_layout],
        push_constant_ranges: &[],
    });

    let pipeline = device.create_compute_pipeline(&wgpu::ComputePipelineDescriptor {
        label: Some("Compute Pipeline"),
        layout: Some(&pipeline_layout),
        module: &shader,
        entry_point: Some("main"),
        compilation_options: Default::default(),
        cache: None,
    });



    // 4. Dispatch compute
    let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
        label: Some("Compute Encoder"),
    });

    {
        let mut cpass = encoder.begin_compute_pass(&wgpu::ComputePassDescriptor { label: Some("Compute Pass"), timestamp_writes: None });
        cpass.set_pipeline(&pipeline);
        cpass.set_bind_group(0, &bind_group, &[]);
        // Workgroups: choose ceil_div(len, 64)
        let wg_size = 64u32;
        let n = input.len() as u32;
        let x = (n + wg_size - 1) / wg_size;
        cpass.dispatch_workgroups(x, 1, 1);
    }

    // Copy output to download buffer
    encoder.copy_buffer_to_buffer(&output_data_buffer, 0, &download_buffer, 0, size_bytes);

    // Submit
    queue.submit(Some(encoder.finish()));

    // 5. Read back results
    let buffer_slice = download_buffer.slice(..);
    let (sender, receiver) = futures_intrusive::channel::shared::oneshot_channel();
    buffer_slice.map_async(wgpu::MapMode::Read, move |v| { sender.send(v).unwrap(); });
    device.poll(wgpu::PollType::Wait).unwrap();
    receiver.receive().await.unwrap().unwrap();

    let data = buffer_slice.get_mapped_range();
    let result: Vec<u32> = bytemuck::cast_slice(&data).to_vec();
    drop(data);
    //staging_dst.unmap();

    Ok(result)
}

/// Blocking helper that runs the async example and returns the doubled values.
pub fn run_compute_example_blocking(input: &[u32]) -> anyhow::Result<Vec<u32>> {
    pollster::block_on(run_compute_example_async(input))
}

// Re-export commonly used deps for convenience in examples.
pub mod prelude {
    pub use wgpu;
}

// Utilities needed by wgpu buffer init
mod util {
    pub use wgpu::util::DeviceExt;
}

use util::*;
