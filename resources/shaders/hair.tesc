#version 460 core

layout (vertices = 16) out;

void main(void) {

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 64.0; // Amount of hair to generate
        gl_TessLevelOuter[1] = 8.0;  // Amount of division per hair 
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
