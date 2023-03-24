#version 410 core

uniform mat4 model;
uniform mat4 view;

in GS_OUT 
{
    vec3 normal;
} fs_in;


out vec4 FragColor;


void main()
{
    vec3 viewSpaceLightDir = vec3(view * vec4(normalize(vec3(0.0, 1.0, 1.0)), 0.0));
    vec3 viewSpaceNormal = normalize(fs_in.normal);

    vec3 albedo = vec3(0.8, 0.8, 0.8);

    vec3 color = albedo * vec3(max(0.0, dot(viewSpaceNormal, viewSpaceLightDir)));
    
    FragColor = vec4(color, 1.0);
}