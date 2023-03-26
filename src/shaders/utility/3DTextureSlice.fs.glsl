#version 460 core

in vec2 vScreenCoords;


uniform sampler3D uInputTexture;
uniform float uDepth = 0.0;


out vec4 FragColor;


void main()
{
	FragColor = texture3D(uInputTexture, vec3(vScreenCoords, uDepth));
}