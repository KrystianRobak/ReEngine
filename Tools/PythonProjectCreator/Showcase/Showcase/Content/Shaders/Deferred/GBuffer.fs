#version 460 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform vec3 uAlbedo = vec3(1.0);
uniform float uMetallic = 0.0;
uniform float uRoughness = 0.5;
uniform bool uUseTextures = false;

void main()
{    
    // 1. Position + Metallic
    gPosition.rgb = FragPos;
    gPosition.a = uMetallic;

    // 2. Normal
    vec3 N = normalize(Normal);
    if (uUseTextures) {
        // Simple normal mapping check (requires loaded texture)
        vec3 normalMapValue = texture(texture_normal1, TexCoords).rgb;
        if(length(normalMapValue) > 0.1) {
            N = normalize(normalMapValue * 2.0 - 1.0);   
            N = normalize(TBN * N);
        }
    }
    gNormal.rgb = N;
    gNormal.a = 1.0;

    // 3. Albedo + Roughness
    if (uUseTextures) {
        gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    } else {
        gAlbedoSpec.rgb = uAlbedo;
    }
    gAlbedoSpec.a = uRoughness;
}