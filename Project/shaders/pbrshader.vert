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
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec3 outTangent;

void main() 
{
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);
    outPos = vec3(push.model * vec4(inPosition, 1.0));

    outColor = inColor;

    outNormal = mat3(push.model) * normalize(inNormal);

    outTexCoord = inTexCoord;

    outTangent = mat3(push.model) * normalize(inTangent);
}