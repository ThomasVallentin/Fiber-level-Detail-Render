#version 460 core

in vec2 vScreenCoords;


const float PI = 3.14195265;


uniform int   uPlyCount = 3;
uniform float uPlyRadius = 1.75;
uniform float uPlyAngle = PI * 0.5;
uniform float uDensityE = 0.25;
uniform float uDensityB = 0.75;


out vec4 FragColor;


float sampleFiberDensity(float e, float B, float R)
{
    return R > 1.0 ? 0.0 : (1.0 - 2.0 * e) * pow((e - pow(e, R)) / (e - 1.0), B) + e;
}

float sampleYarnDensity(vec2 uv, int plyCount, float plyRadius, float e, float B, float plyAngle)
{
    vec2 yarnCenter = vec2(2.0, 2.0);

    float density = 0.0;
    for (int plyIndex = 0 ; plyIndex < plyCount ; plyIndex++)
    {
        float thetaPly = 2.0 * PI * float(plyIndex) / float(plyCount) + plyAngle;

        vec2 scale = vec2(1.3, 1.0);
        mat2 yarnToPly = mat2(cos(thetaPly), -sin(thetaPly), sin(thetaPly), cos(thetaPly));
        vec2 plyCenter = yarnCenter + vec2(cos(thetaPly), sin(thetaPly)) * 0.35 * plyRadius;
        vec2 diff = yarnToPly * (uv - plyCenter) * scale;
        density += sampleFiberDensity(e, B, length(diff));
    }
    
    return density;
}


void main()
{
    vec2  uv = vScreenCoords * 4.0;
    // Output to screen
    float density = sampleYarnDensity(uv, 
                                      uPlyCount, 
                                      uPlyRadius, 
                                      uDensityE, 
                                      uDensityB,
                                      uPlyAngle);
    FragColor = vec4(vec3(density), 1.0);
}
