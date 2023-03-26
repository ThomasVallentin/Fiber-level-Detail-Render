#version 460 core

out vec2 vScreenCoords;

// Drawing a single triangle that covers all the screen with clean screen coords 
// without requiring any vertex buffer
void main(void) {
    vScreenCoords = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(vScreenCoords * 2.0 + -1.0, 0.0, 1.0);
}
