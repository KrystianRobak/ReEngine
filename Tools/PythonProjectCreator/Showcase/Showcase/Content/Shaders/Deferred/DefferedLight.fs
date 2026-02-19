#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D shadowMap;

struct Light {
    int type; // 0 = Directional, 1 = Point, 2 = Spot
    vec3 Position;
    vec3 Direction;
    vec3 Color;
    float Intensity;

    float Constant;
    float Linear;
    float Quadratic;

    float CutOff;
    float OuterCutOff;
};

#define MAX_LIGHTS 32
uniform int  uLightCount;
uniform Light lights[MAX_LIGHTS];

uniform vec3 viewPos;
uniform mat4 lightSpaceMatrix;

// uHasShadow: true if the C++ produced a valid shadow matrix (real or fallback light).
uniform bool uHasShadow;
// uShadowCasterIndex: which lights[] entry owns the shadow map.
// Allows shadows on spotlights and directional lights equally, not just type==0.
uniform int uShadowCasterIndex;

const float PI = 3.14159265359;

// ---------------------------------------------------------------------------
// Shadow
// ---------------------------------------------------------------------------
float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // Outside the far plane -> no shadow
    if (projCoords.z > 1.0)
        return 0.0;

    // Outside the XY bounds of the shadow map -> no shadow
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    // BIAS NOTE: bias lives in NDC depth space [0,1] which maps linearly to
    // [near, far] world units. With far=1000 a bias of 0.001 = 1 world unit.
    //
    // BUG FIX #2: The previous minimum bias of 0.0002–0.002 (0.2–2 world units)
    // was still large enough to swallow the shadow for objects close to the floor.
    // For a platform elevated only 1 unit above the floor with far=1000:
    //   depth diff = 1/1000 = 0.001 NDC
    //   old min bias = 0.0002 → OK, but old max was 0.002 → shadow lost for tilted surfs
    //
    // The real acne prevention now comes from glPolygonOffset(3, 6) applied during
    // the shadow-map render pass (hardware-level depth push). The shader bias only
    // needs to cover floating-point sampling imprecision in the texture lookup.
    // With 32-bit float depth and far=1000, precision at depth ~0.2 is ~1e-7.
    // We use 0.0001 minimum (= 0.1 world units) as a generous safety margin.
    float cosTheta = max(dot(N, L), 0.0);
    // Slope-scale: grazing surfaces need slightly more margin (one texel covers more depth)
    float bias = max(0.0005 * (1.0 - cosTheta), 0.0001);
    bias = clamp(bias, 0.0001, 0.0005);

    // PCF — 3×3 tap for soft edges
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

// ---------------------------------------------------------------------------
// PBR — Cook-Torrance
// ---------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a    = roughness * roughness;
    float a2   = a * a;
    float NdH  = max(dot(N, H), 0.0);
    float denom = (NdH * NdH * (a2 - 1.0) + 1.0);
    return a2 / max(PI * denom * denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / max(NdotV * (1.0 - k) + k, 0.0000001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdV = max(dot(N, V), 0.0);
    float NdL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdV, roughness) * GeometrySchlickGGX(NdL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
void main()
{
    vec4 WorldPosSample = texture(gPosition, TexCoords);
    vec3 WorldPos  = WorldPosSample.rgb;
    float Metallic = WorldPosSample.a;

    vec4 NormalSample = texture(gNormal, TexCoords);
    vec3 Normal = normalize(NormalSample.rgb);

    vec4 AlbedoSample = texture(gAlbedoSpec, TexCoords);
    vec3 Albedo    = AlbedoSample.rgb;
    float Roughness = AlbedoSample.a;

    // Discard background (nothing wrote to gPosition here)
    if (length(WorldPos) == 0.0) {
        FragColor = vec4(0.1, 0.1, 0.1, 1.0);
        return;
    }

    vec3 N  = Normal;
    vec3 V  = normalize(viewPos - WorldPos);
    vec3 F0 = mix(vec3(0.04), Albedo, Metallic);

    // Pre-compute shadow once per fragment (outside the light loop).
    // uShadowCasterIndex tells us which lights[] entry owns the shadow map, so we
    // use that light's direction for slope-based bias. This works for directional
    // lights AND spotlights (the two types that typically use a single shadow map).
    float sceneShadow = 0.0;
    if (uHasShadow && uLightCount > 0)
    {
        vec4 fragPosLightSpace = lightSpaceMatrix * vec4(WorldPos, 1.0);

        int casterIdx = clamp(uShadowCasterIndex, 0, uLightCount - 1);
        vec3 shadowLightDir = normalize(-lights[casterIdx].Direction);
        sceneShadow = ShadowCalculation(fragPosLightSpace, N, shadowLightDir);
    }

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < uLightCount; ++i)
    {
        vec3 L;
        float attenuation = 1.0;

        if (lights[i].type == 0) // Directional
        {
            L = normalize(-lights[i].Direction);
        }
        else // Point or Spot
        {
            L = normalize(lights[i].Position - WorldPos);
            float dist = length(lights[i].Position - WorldPos);
            attenuation = 1.0 / (lights[i].Constant
                                + lights[i].Linear    * dist
                                + lights[i].Quadratic * dist * dist);
        }

        vec3 H       = normalize(V + L);
        vec3 radiance = lights[i].Color * lights[i].Intensity * attenuation;

        // Spot cone soft edges
        if (lights[i].type == 2)
        {
            float theta   = dot(L, normalize(-lights[i].Direction));
            float epsilon = lights[i].CutOff - lights[i].OuterCutOff;
            float spotInt = clamp((theta - lights[i].OuterCutOff) / epsilon, 0.0, 1.0);
            radiance *= spotInt;
        }

        float NDF = DistributionGGX(N, H, Roughness);
        float G   = GeometrySmith(N, V, L, Roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3  specular = (NDF * G * F) / max(4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0), 0.0001);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - Metallic);

        float NdotL = max(dot(N, L), 0.0);

        // Apply shadow only to the light that owns the shadow map (uShadowCasterIndex).
        // This correctly handles directional lights AND spotlights. Point lights are
        // omnidirectional so an ortho shadow map doesn't represent them; skip those.
        bool isShadowCaster = (i == uShadowCasterIndex) && (lights[i].type != 1);
        float lightShadow = isShadowCaster ? sceneShadow : 0.0;

        Lo += (kD * Albedo / PI + specular) * radiance * NdotL * (1.0 - lightShadow);
    }

    vec3 ambient = vec3(0.03) * Albedo;
    vec3 color   = ambient + Lo;

    // Reinhard tonemapping + gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}