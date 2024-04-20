#version 450

layout(push_constant) uniform PushConstants 
{
    mat4 model; 
} push;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 model;
    mat4 proj;
    mat4 view; 
} vp;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;

void main() 
{
    gl_Position = vp.proj * vp.view * push.model * vec4(inPosition, 1.0);
    vec4 tNormal =  push.model * vec4(inNormal, 0.0);
    fragNormal = normalize(tNormal.xyz); // interpolation of normal attribute in fragment shader.
    fragColor = inColor; // interpolation of color attribute in fragment shader.
}