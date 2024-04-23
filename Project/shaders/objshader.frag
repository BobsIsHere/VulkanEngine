#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

const vec3 lightPosition = vec3(1.2, 1.0, 2.0);
const vec3 lightColor = vec3(0.7, 0.7, 1.0);

void main() 
{
    if(length(fragNormal) == 0.0)
    {
        outColor = vec4(fragColor, 1.0);
        return;
    }

    const vec3 lightDirection = normalize(vec3(0.0, -1.0, -1.0));

    vec3 result = fragColor * lightColor * max(dot(normalize(fragNormal), lightDirection), 0.0);

    // Output color
    outColor = vec4(result, 1.0);
}