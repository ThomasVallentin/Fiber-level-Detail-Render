#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 6) out;


in TS_OUT 
{
    int globalFiberIndex;
    vec3 yarnNormal;
    vec3 yarnTangent;
    vec3 yarnBitangent;
} gs_in[]; 


uniform mat4 view; 
uniform mat4 projection;


out GS_OUT 
{
    vec3 normal;
} gs_out;


void main() {    
    float thickness = 0.0075;

    // if (gs_in[0].globalFiberIndex % 22 == 0) 
    //     thickness = 10.0;

    vec3 pntA = gl_in[0].gl_Position.xyz;
    vec3 pntB = gl_in[1].gl_Position.xyz;
    vec3 tangentA = normalize(vec3(view * vec4(gs_in[0].yarnTangent, 0.0)));
    vec3 tangentB = normalize(vec3(view * vec4(gs_in[1].yarnTangent, 0.0)));

    vec3 toCamera = normalize(-pntA);
    vec3 frontFacingBitangentA = cross(toCamera, tangentA);
    vec3 viewSpaceNormalA = cross(tangentA, frontFacingBitangentA);

    vec3 frontFacingBitangentB = cross(toCamera, tangentB);
    vec3 viewSpaceNormalB = cross(tangentB, frontFacingBitangentB);

    // Top left
    gs_out.normal = -frontFacingBitangentB;
    gl_Position = projection * vec4(pntB - frontFacingBitangentB * thickness, 1.0);
    EmitVertex();

    // Bottom left
    gs_out.normal = -frontFacingBitangentA;
    gl_Position = projection * vec4(pntA - frontFacingBitangentA * thickness, 1.0);
    EmitVertex();

    // Top middle
    gs_out.normal = viewSpaceNormalB;
    gl_Position = projection * vec4(pntB, 1.0);
    EmitVertex();

    // Bottom middle
    gs_out.normal = viewSpaceNormalA;
    gl_Position = projection * vec4(pntA, 1.0);
    EmitVertex();

    // Top right
    gs_out.normal = frontFacingBitangentB;
    gl_Position = projection * vec4(pntB + frontFacingBitangentB * thickness, 1.0);
    EmitVertex();

    // Bottom right
    gs_out.normal = frontFacingBitangentA;
    gl_Position = projection * vec4(pntA + frontFacingBitangentA * thickness, 1.0);
    EmitVertex();
}  