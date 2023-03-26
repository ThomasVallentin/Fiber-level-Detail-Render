#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;


// == Inputs ==

in TS_OUT 
{
    int globalFiberIndex;
    vec3 yarnCenter;
    vec3 fiberNormal;
    vec3 yarnTangent;
} gs_in[]; 


// == Uniforms ==

uniform mat4 uViewMatrix; 
uniform mat4 uProjMatrix;

uniform int uPlyCount = 3;


// == Outputs ==

out GS_OUT 
{
    flat int fiberIndex;
    vec3 position;
    vec3 normal;
    float distanceFromYarnCenter;
} gs_out;


void main() 
{    
    float thickness = 0.003;

    gs_out.fiberIndex = gs_in[0].globalFiberIndex;
    if (gs_out.fiberIndex < uPlyCount) 
        thickness *= 10.0;

    vec3 pntA = gl_in[0].gl_Position.xyz;
    vec3 pntB = gl_in[1].gl_Position.xyz;
    vec3 tangentA = normalize(pntB - pntA);
    vec3 tangentB = tangentA;

    vec3 toCameraA = normalize(-pntA);
    vec3 frontFacingBitangentA = normalize(cross(toCameraA, tangentA));
    vec3 normalA     = gs_in[0].fiberNormal;
    vec3 yarnCenterA = gs_in[0].yarnCenter;

    vec3 toCameraB = normalize(-pntB);
    vec3 frontFacingBitangentB = normalize(cross(toCameraB, tangentB));
    vec3 normalB     = gs_in[1].fiberNormal;
    vec3 yarnCenterB = gs_in[1].yarnCenter;

    // Top left
    vec3 vertex = pntB - frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalB;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterB);
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();

    // Bottom left
    vertex = pntA - frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalA;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterA);
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();

    // Top right
    vertex = pntB + frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalB;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterB);
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();

    // Bottom right
    vertex = pntA + frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalA;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterA);
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();
}  