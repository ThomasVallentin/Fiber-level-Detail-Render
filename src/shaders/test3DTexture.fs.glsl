#version 460 core

in vec2 vScreenCoords;

uniform sampler3D uTexture3D;
uniform float uTime = 1.0;

out vec4 FragColor;

void main()
{
	FragColor = texture3D(uTexture3D, vec3(vScreenCoords, uTime));
	// FragColor = vec4(vScreenCoords, 0.0, 1.0);
}