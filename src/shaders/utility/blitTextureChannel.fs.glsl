#version 330

// == Inputs ==================================================================

in vec2 vScreenCoords;


// == Uniforms ================================================================

uniform sampler2D uInput;
uniform int uChannel = 0;


// == Outputs =================================================================

out vec4 fColor;


// == Shader evaluation =======================================================

void main()
{
    fColor = vec4(vec3(texture(uInput, vScreenCoords)[uChannel]), 1.0);
}
