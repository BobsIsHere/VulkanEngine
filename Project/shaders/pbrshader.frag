#version 450

// -------------------- VARIABLES --------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D diffuseSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D glossSampler;
layout(binding = 4) uniform sampler2D specularSampler;

// -------------------- FUNCTIONS --------------------

vec3 Lambert(vec3 kd, vec3 cd)
{
	const vec3 rho = cd * kd;
    return rho / 3.14159265359;
}

vec3 Phong(vec3 lightDir, float reflection, float exponent, vec3 v, vec3 n)
{
    float dp = dot(n, lightDir);
    vec3 reflect = lightDir - (2.0 * dp * n);

    float cosAlpha = max(dot(reflect, v), 0.f);
    float specular = reflection * pow(cosAlpha, exponent);

    return vec3(specular, specular, specular);
}

// -------------------- MAIN --------------------

void main() 
{
    vec3 lightDirection = vec3(0.577, 0.577, 0.577);
    vec3 viewDirection = normalize(-inPosition);
    vec3 radiance = vec3(7.0, 7.0, 7.0);
    vec3 ambient = vec3(0.03, 0.03, 0.03);
    float shininess = 25.0;
    float lightIntensity = 2.0;

    vec3 diffuseTexture = texture(diffuseSampler, fragTexCoord).rgb;
    vec3 normalTexture = texture(normalSampler, fragTexCoord).rgb;
    float glossTexture = texture(glossSampler, fragTexCoord).r;
    float specularTexture = texture(specularSampler, fragTexCoord).r;
    
    vec3 binormal = cross(fragNormal, fragTangent);
    mat3 tangentSpaceAxis = mat3(fragTangent, binormal, fragNormal);

    vec3 sampledNormal = 2.f * normalTexture - 1.f;
    sampledNormal = normalize(tangentSpaceAxis * sampledNormal);

    float observedArea = dot(sampledNormal, -lightDirection);
    if (observedArea <= 0.0)
	{
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
	}

    vec3 exponent = vec3(glossTexture * shininess);

    vec3 lambert = Lambert(radiance, diffuseTexture);
    vec3 phong = Phong(lightDirection, specularTexture, glossTexture * 25.0, -viewDirection, sampledNormal);

    outColor = vec4(lambert, 1.0);
    //outColor = vec4(((lambert * lightIntensity) + phong + ambient) * observedArea, 1.0);
}