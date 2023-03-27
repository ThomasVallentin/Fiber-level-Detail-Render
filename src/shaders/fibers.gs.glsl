#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;


// == Inputs ==

in TS_OUT 
{
    int globalFiberIndex;
    vec3 yarnCenter;
    vec3 yarnNormal;
    vec3 yarnTangent;
    vec3 fiberNormal;
    float plyRotation;
} gs_in[]; 


// == Uniforms ==

uniform mat4 uViewMatrix; 
uniform mat4 uProjMatrix;

uniform int uPlyCount = 3;

uniform vec3 uLightDirection;

// == Outputs ==

out GS_OUT 
{
    flat int fiberIndex;
    vec3 position;
    vec3 normal;

    float distanceFromYarnCenter;

    vec2 selfShadowSample;
    float plyRotation;
} gs_out;


void main() 
{    
    float thickness = 0.003;

    gs_out.fiberIndex = gs_in[0].globalFiberIndex;
    if (gs_out.fiberIndex < uPlyCount) 
        thickness *= 10.0;

    vec3 pntA = gl_in[0].gl_Position.xyz;
    vec3 pntB = gl_in[1].gl_Position.xyz;
    vec3 fiberTangent = normalize(pntB - pntA);

    vec3 toCameraA = normalize(-pntA);
    vec3 frontFacingBitangentA = normalize(cross(toCameraA, fiberTangent));
    vec3 normalA     = gs_in[0].fiberNormal;
    vec3 yarnCenterA = gs_in[0].yarnCenter;

    vec3 toCameraB = normalize(-pntB);
    vec3 frontFacingBitangentB = normalize(cross(toCameraB, fiberTangent));
    vec3 normalB     = gs_in[1].fiberNormal;
    vec3 yarnCenterB = gs_in[1].yarnCenter;

    // Self shadows
    vec3 toLight = normalize(-uLightDirection);
    vec3 yarnTangentA = gs_in[0].yarnTangent;
    vec3 bitangentToLightA = -normalize(cross(yarnTangentA, toLight));
    vec3 normalToLightA = cross(bitangentToLightA, yarnTangentA);
    vec2 selfShadowSampleA = (transpose(mat3(normalToLightA, bitangentToLightA, yarnTangentA)) * (pntA - yarnCenterA)).xy;

    vec3 yarnTangentB = gs_in[1].yarnTangent;
    vec3 bitangentToLightB = -normalize(cross(yarnTangentB, toLight));
    vec3 normalToLightB = cross(bitangentToLightB, yarnTangentB);
    vec2 selfShadowSampleB = (transpose(mat3(normalToLightB, bitangentToLightB, yarnTangentB)) * (pntB - yarnCenterB)).xy;

    float plyRotationA = gs_in[0].plyRotation;
    float plyRotationB = gs_in[1].plyRotation;

    // Top left
    vec3 vertex = pntB - frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalB;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterB);
    gs_out.selfShadowSample = selfShadowSampleB;
    gs_out.plyRotation = plyRotationB;
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();

    // Bottom left
    vertex = pntA - frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalA;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterA);
    gs_out.selfShadowSample = selfShadowSampleA;
    gs_out.plyRotation = plyRotationA;
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();

    // Top right
    vertex = pntB + frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalB;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterB);
    gs_out.selfShadowSample = selfShadowSampleB;
    gs_out.plyRotation = plyRotationB;    
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();

    // Bottom right
    vertex = pntA + frontFacingBitangentA * thickness;
    gs_out.position = vertex;
    gs_out.normal = normalA;
    gs_out.distanceFromYarnCenter = distance(vertex, yarnCenterA);
    gs_out.selfShadowSample = selfShadowSampleA;
    gs_out.plyRotation = plyRotationA;
    gl_Position = uProjMatrix * vec4(vertex, 1.0);
    EmitVertex();
}  