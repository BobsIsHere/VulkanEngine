#version 450

//--------------------------------------------
//   Variables
//--------------------------------------------

layout(location = 0) in vec3 outPosition;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D glossSampler;
layout(binding = 4) uniform sampler2D specularSampler;

layout(push_constant) uniform PushConstants
{
    layout(offset = 64) int renderingMode;
} pushRenderingMode;

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
    const vec3 viewDirection = normalize(-outPosition);
    const float lightIntensity = 10.f; // Increased light intensity
    const float shininess = 25.f;

    vec3 diffuseTexture = texture(diffuseSampler, fragTexCoord).rgb;
    vec3 normalTexture = texture(normalSampler, fragTexCoord).rgb;
    vec3 glossTexture = texture(glossSampler, fragTexCoord).rgb;
    vec3 specularTexture = texture(specularSampler, fragTexCoord).rgb;
    
    const vec3 binormal = cross(fragNormal, fragTangent);
    const mat3 tangentSpaceAxis = mat3(fragTangent, binormal, fragNormal);

    // Sample from normal map and transform it to tangent space
    vec3 sampledNormal = 2.f * normalTexture - 1.f;
    sampledNormal = normalize(tangentSpaceAxis * sampledNormal);

    // Modified observed area calculation for more even light distribution
    const float observedArea = 0.5f + 0.5f * max(dot(sampledNormal, lightDirection), 0.f);

    // Lambertian diffuse shading term
    vec3 diffuseTerm = diffuseTexture / 3.14159265359; // Adjusted diffuse term scaling

    // Phong specular reflection term
    vec3 phong = PhongReflection(specularTexture.r, glossTexture.r * shininess, lightDirection, viewDirection, sampledNormal); // Adjusted Phong term scaling

    // Combine terms and ensure minimum brightness level
    vec3 result;

     // Albedo + Lambert shading
    if (pushRenderingMode.renderingMode == 0)
    {
        result = diffuseTerm;
    }
    // Normal
    else if (pushRenderingMode.renderingMode == 1)
    {
        result = sampledNormal;
    }
    // Specular
    else if (pushRenderingMode.renderingMode == 2)
    {
        result = phong;
    }
    // Combined
    else if (pushRenderingMode.renderingMode == 3)
    {
        result = ((diffuseTerm * lightIntensity) + phong) * observedArea;
    }

    outColor = vec4(result, 1.0);
}
