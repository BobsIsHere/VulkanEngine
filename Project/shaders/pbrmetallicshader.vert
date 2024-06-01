#version 450

layout(push_constant) uniform PushConstants 
{
    mat4 model; 
} push;

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 proj;
    mat4 view; 
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;
layout(location = 3) out vec3 outViewDirection;

void main() 
{
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);
    outViewDirection = normalize(vec3(gl_Position) - vec3(ubo.view[1][0], ubo.view[1][1], ubo.view[1][2]));

    outNormal = normalize(mat3(push.model) * inNormal);

    outTexCoord = inTexCoord;

    outTangent = normalize(mat3(push.model) * inTangent);
}