// tessellation control shader
#version 410 core

// specify number of control points per patch output
// this value controls the size of the input and output arrays
layout (vertices=4) out;

void main()
{
    // ----------------------------------------------------------------------
    // pass attributes through
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // ----------------------------------------------------------------------
    // invocation zero controls tessellation levels for the entire patch
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 64;
        gl_TessLevelOuter[1] = 64;
    }
}