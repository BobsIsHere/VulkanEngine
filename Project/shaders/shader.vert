#version 450

layout(push_constant) uniform PushConstants
{
	mat4 model;
} push;

layout(set = 0, binding = 0) uniform ViewProjection 
{
    mat4 proj;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main() 
{
    gl_Position = ubo.proj * push.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
