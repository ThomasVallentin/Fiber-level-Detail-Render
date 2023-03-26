#version 410 core
layout (lines) in;
layout (line_strip, max_vertices = 6) out;


// == Inputs ==

in TS_OUT 
{
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} gs_in[]; 


// == Uniforms ==

uniform mat4 uViewMatrix; 
uniform mat4 uProjMatrix;

uniform int uPlyCount = 3;

uniform vec3 uLightDirection;

// == Outputs ==

out GS_OUT 
{
    vec3 position;
    vec3 normal;
    vec3 texCoord;
} gs_out;


void main() 
{    
    float thickness = 0.025;

    vec4 position  = gl_in[0].gl_Position;
    vec3 normal    = gs_in[0].normal;
    vec3 tangent   = gs_in[0].tangent;

    vec3 toLight = normalize(-uLightDirection);
    vec3 bitangentToLight = -normalize(cross(tangent, toLight));
    vec3 normalToLight = cross(bitangentToLight, tangent);

    gs_out.color = vec3(1, 0, 0);
    gs_out.position = position.xyz;
    gs_out.normal = normal;
    gl_Position = uProjMatrix * vec4(gs_out.position, 1.0);
    EmitVertex();
    gs_out.position =  position.xyz + normalToLight * thickness;
    gs_out.normal = normal;
    gl_Position = uProjMatrix * vec4(gs_out.position, 1.0);
    EmitVertex();
    EndPrimitive();

    gs_out.color = vec3(0, 1, 0);
    gs_out.position =  position.xyz;
    gs_out.normal = normal;
    gl_Position = uProjMatrix * vec4(gs_out.position, 1.0);
    EmitVertex();
    gs_out.position =  position.xyz + tangent * thickness;
    gs_out.normal = normal;
    gl_Position = uProjMatrix * vec4(gs_out.position, 1.0);
    EmitVertex();
    EndPrimitive();

    gs_out.color = vec3(0, 0, 1);
    gs_out.position =  position.xyz;
    gs_out.normal = normal;
    gl_Position = uProjMatrix * vec4(gs_out.position, 1.0);
    EmitVertex();
    gs_out.position =  position.xyz + bitangentToLight * thickness;
    gs_out.normal = normal;
    gl_Position = uProjMatrix * vec4(gs_out.position, 1.0);
    EmitVertex();
    EndPrimitive();
}
