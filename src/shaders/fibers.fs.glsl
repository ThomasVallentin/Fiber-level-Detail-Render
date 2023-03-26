#version 410 core


// == Inputs ==

in GS_OUT 
{
    flat int fiberIndex;
    vec3 position;
    vec3 normal;
    float distanceFromYarnCenter;
} fs_in;


// == Uniforms ==

uniform mat4 view;

uniform int uPlyCount = 3;
uniform float R_ply;
uniform float Rmin; 
uniform bool uUseAmbientOcclusion;

// Shadow mapping
uniform sampler2D uShadowMap;
uniform float uShadowIntensity = 0.85;
uniform mat4 uViewToLightMatrix;
uniform bool uReceiveShadows = true;
uniform bool uSmoothShadows = true;

// == Outputs ==

out vec4 FragColor;


vec3 sampleAlbedo(vec2 texCoord)
{
    return vec3(0.8);
    // return uUseAlbedoTexture ? texture(uAlbedoTexture, texCoord).rgb : uAlbedoColor;
}

float sampleAmbientOcclusion()
{
    return uUseAmbientOcclusion ? min(1.0, fs_in.distanceFromYarnCenter / R_ply) : 1.0;
}


float sampleShadows(vec4 lightSpacePosition)
{
    if (!uReceiveShadows)
        return 0.0;
    
    vec3 lightProjectedPos = lightSpacePosition.xyz / lightSpacePosition.w;
    lightProjectedPos = lightProjectedPos * 0.5 + 0.5;
    float fragmentDepth = lightProjectedPos.z;

    if (fragmentDepth > 1.0)
        return 0.0;

    if (uSmoothShadows)
    {
        float shadowDepth;
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(-texelSize.x, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(-texelSize.x, 0.0)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(-texelSize.x, texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(0.0, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(0.0, texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(texelSize.x, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(texelSize.x, 0.0)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(texelSize.x, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;

        return (shadow / 9.0);
    }

    float shadowDepth = texture(uShadowMap, lightProjectedPos.xy).r;
    return fragmentDepth > shadowDepth ? uShadowIntensity : 0.0;
}


void main()
{
    vec3 viewSpaceLightDir = vec3(view * vec4(normalize(vec3(0.0, 1.0, 1.0)), 0.0));
    vec3 viewSpaceNormal = normalize(fs_in.normal);

    vec3 albedo = sampleAlbedo(vec2(0.0, 0.0));
    float ambientOcclusion = sampleAmbientOcclusion();
    float shadowMask = 1.0 - sampleShadows(uViewToLightMatrix * vec4(fs_in.position, 1.0));

    // vec3 color = vec3(shadowMask);
    vec3 color = shadowMask * ambientOcclusion * albedo;
    // vec3 color = shadowMask * ambientOcclusion * albedo * vec3(max(0.0, dot(viewSpaceNormal, viewSpaceLightDir)));

    FragColor = vec4(vec3(color), 1.0);
}