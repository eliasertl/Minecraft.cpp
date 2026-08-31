struct CameraUniforms {
    viewProjection: mat4x4f,
};

struct ChunkUniforms {
    transform: mat4x4f,
};

@group(0) @binding(0)
var<uniform> uCam: CameraUniforms;

@group(1) @binding(0)
var<uniform> uChunk: ChunkUniforms;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) color: vec3f,
};

struct VertexOutput {
    @builtin(position) clip_position: vec4f,
    @location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.clip_position = uCam.viewProjection * uChunk.transform * vec4f(in.position, 1.0);
    out.color = in.color;
    return out;
}

struct FragmentOutput {
    @location(0) color: vec4f,
};

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    var out: FragmentOutput;
    out.color = vec4f(in.color, 1.0);
    return out;
}