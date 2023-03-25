#version 410 core

uniform mat4 view;

uniform float R_ply;
uniform float Rmin; 
uniform bool uUseAmbientOcclusion;

in GS_OUT 
{
    vec3 normal;
    float distanceFromYarnCenter;
} fs_in;


out vec4 FragColor;


float sampleAmbientOcclusion()
{
    return uUseAmbientOcclusion ? min(1.0, fs_in.distanceFromYarnCenter / (R_ply + Rmin * 0.5)) : 1.0;
}


void main()
{
    vec3 viewSpaceLightDir = vec3(view * vec4(normalize(vec3(0.0, 1.0, 1.0)), 0.0));
    vec3 viewSpaceNormal = normalize(fs_in.normal);

    vec3 albedo = vec3(0.8);
    float ambientOcclusion = sampleAmbientOcclusion();

    vec3 color = ambientOcclusion * albedo * vec3(max(0.0, dot(viewSpaceNormal, viewSpaceLightDir)));
    
    FragColor = vec4(vec3(color), 1.0);
}