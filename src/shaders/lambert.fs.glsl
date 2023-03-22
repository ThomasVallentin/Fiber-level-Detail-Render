#version 460 core

// ========================================================

in VertexData
{
    vec3 viewSpacePosition;    
    vec4 worldSpacePosition;
    vec3 normal;
    vec2 texCoord;
} inVertex;

// ========================================================

uniform sampler2D uAlbedoTexture;
uniform vec3 uAlbedoColor = vec3(0.8, 0.8, 0.8);
uniform bool uUseAlbedoTexture = false;

uniform sampler2D uShadowMap;
uniform mat4 uLightSpaceMatrix;
uniform bool uReceiveShadows = true;
uniform bool uSmoothShadows = true;

// ========================================================

out vec4 fColor;

// ========================================================

vec3 getAlbedo(vec2 texCoord)
{
    return uUseAlbedoTexture ? texture(uAlbedoTexture, texCoord).rgb : uAlbedoColor;
}

float sampleShadows(vec4 lightSpacePosition)
{
    if (!uReceiveShadows)
        return 1.0;
    
    vec3 lightProjectedPos = lightSpacePosition.xyz / lightSpacePosition.w;
    lightProjectedPos = lightProjectedPos * 0.5 + 0.5;
    float fragmentDepth = lightProjectedPos.z;

    if (fragmentDepth > 1.0)
        return 1.0;

    if (uSmoothShadows)
    {
        float shadowDepth;
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(-texelSize.x, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(-texelSize.x, 0.0)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(-texelSize.x, texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(0.0, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(0.0, texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(texelSize.x, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(texelSize.x, 0.0)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;
        shadowDepth = texture(uShadowMap, lightProjectedPos.xy + vec2(texelSize.x, -texelSize.y)).r;
        shadow += fragmentDepth > shadowDepth ? 0.0 : 1.0;

        return shadow / 9.0;
    }

    float shadowDepth = texture(uShadowMap, lightProjectedPos.xy).r;
    return fragmentDepth > shadowDepth ? 0.0 : 1.0;
}

void main() 
{
    vec3 camDirection = vec3(0.0, 0.0, 1.0);
    vec3 normal = normalize(inVertex.normal);

    vec3 albedo = getAlbedo(inVertex.texCoord);
    float shadowMask = sampleShadows(uLightSpaceMatrix * inVertex.worldSpacePosition);

    vec3 color = shadowMask * albedo * vec3(abs(dot(normal, camDirection)));
    
    fColor = vec4(color, 1.0);
}
