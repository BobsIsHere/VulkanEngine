#version 450

//--------------------------------------------
//   Variables
//--------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D glossSampler;
layout(binding = 4) uniform sampler2D specularSampler;

//--------------------------------------------
//   Lambert Shader
//--------------------------------------------
vec3 LambertShading(const float kd, const vec3 cd)
{
    vec3 lambertDiffuse = (cd * kd) / 3.14159265359;
    return lambertDiffuse;
}

//--------------------------------------------
//   Phong Reflection
//--------------------------------------------
vec3 PhongReflection(float ks, float exponent, vec3 lightVector, vec3 viewVector, vec3 normal)
{    
    const float dotProduct = dot(normal, lightVector);
    const vec3 reflection = lightVector - (2.f * dotProduct * normal);

    const float angle = max(dot(reflection, viewVector), 0.f);
    const float phong = ks * pow(angle, exponent);

    return vec3(phong, phong, phong);
}

//--------------------------------------------
//   Main Function
//--------------------------------------------

void main() 
{
    const vec3 lightDirection = vec3(0.577f, 0.577f, 0.577f);
    const vec3 viewDirection = normalize(-inPosition);
    const float lightIntensity = 7.f;
    const float shininess = 25.f;

    vec3 diffuseTexture = texture(diffuseSampler, fragTexCoord).rgb;
    vec3 normalTexture = texture(normalSampler, fragTexCoord).rgb;
    vec3 glossTexture = texture(glossSampler, fragTexCoord).rgb;
    vec3 specularTexture = texture(specularSampler, fragTexCoord).rgb;
    
    const vec3 binormal = cross(fragNormal, fragTangent);
    const mat3 tangentSpaceAxis = mat3(fragTangent, binormal, fragNormal);

    //sample from normal map and multiply it with matrix
    //change range [0, 1] to [-1, 1]
    vec3 sampledNormal = 2.f * normalTexture - 1.f;
    sampledNormal = normalize(tangentSpaceAxis * sampledNormal);

    const float observedArea = max(dot(sampledNormal, lightDirection), 0.f);

    vec3 diffuseTerm = diffuseTexture / 3.14159265359;
    vec3 phong = PhongReflection(specularTexture.r, glossTexture.r * shininess, lightDirection, viewDirection, sampledNormal);
    vec3 result = ((diffuseTerm * lightIntensity) + phong) * observedArea;

    outColor = vec4(result, 1.0);
}