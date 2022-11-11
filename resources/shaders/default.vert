#version 460 core

layout (location = 0) in vec3 aPosition;

out vec4 vPosition;

void main() {
    vPosition = vec4(aPosition, 1.0);
    gl_Position = vPosition;
}
