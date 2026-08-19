@vertex
fn vs_main(@location(0) in_vertex_position: vec3f) -> @builtin(position) vec4f {
    return vec4(in_vertex_position, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.9, 0.2, 0.1, 1.0);
}