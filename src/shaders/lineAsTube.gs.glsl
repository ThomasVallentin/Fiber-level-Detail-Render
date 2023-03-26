#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 18) out;


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

uniform float uThickness = 0.01;

// == Outputs ==

out GS_OUT 
{
    vec3 position;
    vec3 normal;
    float distanceFromYarnCenter;
} gs_out;


const float PI = 3.14159265;
const int maxTubeDivision = 8;


void main() 
{    
    vec3 pntA = gl_in[0].gl_Position.xyz;
    vec3 pntB = gl_in[1].gl_Position.xyz;

    vec3 tangentA   = gs_in[0].tangent;
    vec3 tangentB   = gs_in[1].tangent;
    vec3 normalA    = gs_in[0].normal;
    vec3 normalB    = gs_in[1].normal;
    vec3 bitangentA = gs_in[0].bitangent;
    vec3 bitangentB = gs_in[1].bitangent;

    float theta;
    vec3 displacement;
    vec3 vertex;
    for (int i = 0 ; i < maxTubeDivision + 1; i++)  // Iterating one more time to close the tube
    {
        theta = 2.0 * PI * i / maxTubeDivision;

        displacement = cos(theta) * normalA + sin(theta) * bitangentA;
        vertex = pntA + displacement * uThickness;
        gs_out.position = vertex;
        gs_out.normal = normalize(displacement);
        gl_Position = uProjMatrix * vec4(vertex, 1.0);
        EmitVertex();

        displacement = cos(theta) * normalB + sin(theta) * bitangentB;
        vertex = pntB + displacement * uThickness;
        gs_out.position = vertex;
        gs_out.normal = normalize(displacement);
        gl_Position = uProjMatrix * vec4(vertex, 1.0);
        EmitVertex();
    }
}  