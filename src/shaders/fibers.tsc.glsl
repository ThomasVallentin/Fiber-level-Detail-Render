// tessellation control shader
#version 410 core

// specify number of control points per patch output
// this value controls the size of the input and output arrays
layout (vertices=2) out;


// == Uniform ==

uniform int uTessLineCount = 64;
uniform int uTessSubdivisionCount = 16;

// == Outputs ==

patch out vec4 pPrevPoint;
patch out vec4 pNextPoint;

void main()
{
    // invocation zero controls tessellation levels for the entire patch
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = uTessLineCount;
        gl_TessLevelOuter[1] = uTessSubdivisionCount;
    
        pPrevPoint = gl_in[0].gl_Position;
        pNextPoint = gl_in[3].gl_Position;
        gl_out[gl_InvocationID].gl_Position = gl_in[1].gl_Position;
    }

    if (gl_InvocationID == 1)
    {
        gl_out[gl_InvocationID].gl_Position = gl_in[2].gl_Position;
    }
}