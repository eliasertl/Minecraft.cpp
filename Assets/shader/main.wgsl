struct CameraUniforms {
    viewProjection: mat4x4f,
};

@group(0) @binding(0)
var<uniform> uCam: CameraUniforms;

@group(0) @binding(1) 
var atlasTexture: texture_2d<f32>;

@group(0) @binding(2)
var atlasSampler: sampler;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
};

struct VertexOutput {
    @builtin(position) clip_position: vec4f,
    @location(0) uv: vec2f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.clip_position = uCam.viewProjection * vec4f(in.position, 1.0);
    out.uv = in.uv;
    return out;
}

struct FragmentOutput {
    @location(0) color: vec4f,
};

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    var out: FragmentOutput;
    out.color = textureSample(atlasTexture, atlasSampler, in.uv);
    return out;
}