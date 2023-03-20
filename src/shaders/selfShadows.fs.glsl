#version 460 core

in vec2 vScreenCoords;

uniform sampler2D uDensityTexture;

out vec4 FragColor;


float sampleDensity(float dist, vec2 texCoords) 
{
    float density = texture(uDensityTexture, texCoords).x;
    return 1.0 - exp(-dist * density);
}


void main()
{
    int screenWidth = textureSize(uDensityTexture, 0).x;
    float stepSize = 1.0 / screenWidth;
    int currentFragmentX = int(vScreenCoords.x * screenWidth);

    float absorbedLight = 0.0;
    for (int i = 0 ; i < currentFragmentX ; i++)
    {
        vec2 texCoords = vec2(float(i) * stepSize, vScreenCoords.y);
        absorbedLight += sampleDensity(stepSize, texCoords);
    }

    FragColor = vec4(vec3(absorbedLight), 1.0);
}
