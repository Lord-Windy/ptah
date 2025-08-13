@group(0) @binding(0)
var<storage, read> in_data: array<u32>;

@group(0) @binding(1)
var<storage, read_write> out_data: array<u32>;

@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let idx = gid.x;
    if (idx < arrayLength(&in_data)) {
        out_data[idx] = in_data[idx] * 2u;
    }
}
