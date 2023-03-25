#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;


in TS_OUT 
{
    int globalFiberIndex;
    vec3 yarnCenter;
    vec3 fiberNormal;
    vec3 yarnTangent;
} gs_in[]; 


uniform mat4 view; 
uniform mat4 projection;

uniform float Rmin;
uniform int uPlyCount = 3;

out GS_OUT 
{
    vec3 normal;
    float distanceFromYarnCenter;
} gs_out;


void main() {    
    float thickness = 0.005;

    // if (gs_in[0].globalFiberIndex < uPlyCount) 
    //     thickness = Rmin;

    vec3 pntA = gl_in[0].gl_Position.xyz;
    vec3 pntB = gl_in[1].gl_Position.xyz;
    vec3 tangentA = normalize(pntB - pntA);
    vec3 tangentB = tangentA;

    vec3 toCameraA = normalize(-pntA);
    vec3 frontFacingBitangentA = normalize(cross(toCameraA, tangentA));
    vec3 normalA = gs_in[0].fiberNormal;

    vec3 toCameraB = normalize(-pntB);
    vec3 frontFacingBitangentB = normalize(cross(toCameraB, tangentB));
    vec3 normalB = gs_in[1].fiberNormal;

    // Top left
    vec3 vertex = pntB - frontFacingBitangentA * thickness;
    gs_out.normal = normalB;
    gs_out.distanceFromYarnCenter = distance(vertex, gs_in[1].yarnCenter);
    gl_Position = projection * vec4(vertex, 1.0);
    EmitVertex();

    // Bottom left
    vertex = pntA - frontFacingBitangentA * thickness;
    gs_out.normal = normalA;
    gs_out.distanceFromYarnCenter = distance(vertex, gs_in[0].yarnCenter);
    gl_Position = projection * vec4(vertex, 1.0);
    EmitVertex();

    // Top right
    vertex = pntB + frontFacingBitangentA * thickness;
    gs_out.normal = normalB;
    gs_out.distanceFromYarnCenter = distance(vertex, gs_in[1].yarnCenter);
    gl_Position = projection * vec4(vertex, 1.0);
    EmitVertex();

    // Bottom right
    vertex = pntA + frontFacingBitangentA * thickness;
    gs_out.normal = normalA;
    gs_out.distanceFromYarnCenter = distance(vertex, gs_in[0].yarnCenter);
    gl_Position = projection * vec4(vertex, 1.0);
    EmitVertex();
}  