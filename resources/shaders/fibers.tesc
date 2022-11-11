#version 460 core

layout (vertices = 4) out;

void main(void) {

        gl_TessLevelOuter[0] = 64.0;  // Amount of fibers to generate
        gl_TessLevelOuter[1] = 16.0;  // Amount of division per fiber 

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
