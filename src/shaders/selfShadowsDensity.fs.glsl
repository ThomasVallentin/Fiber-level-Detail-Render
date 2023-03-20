#version 460 core

in vec2 vScreenCoords;


const float PI = 3.14195265;


uniform int   uPlyCount = 3;
uniform float uPlyRadius = 1.75;
uniform float uPlyAngle = PI * 0.5;
uniform float uDensityE = 0.25;
uniform float uDensityB = 0.75;
uniform float uEN = 1.0; // ellipse scaling factor along Normal
uniform float uEB = 1.0; // ellipse scaling factor along Bitangent


out vec4 FragColor;


float sampleFiberDensity(float e, float B, float R)
{
    return R > 1.0 ? 0.0 : (1.0 - 2.0 * e) * pow((e - pow(e, R)) / (e - 1.0), B) + e;
}

float sampleYarnDensity(vec2 uv, int plyCount, float plyRadius, float plyAngle, float eN, float eB, float e, float B)
{
    vec2 yarnCenter = vec2(2.0, 2.0);

    float density = 0.0;
    for (int plyIndex = 0 ; plyIndex < plyCount ; plyIndex++)
    {
        float thetaPly = 2.0 * PI * float(plyIndex) / float(plyCount) + plyAngle;

        vec2 scale = vec2(eN, eB);
        mat2 plyInverseMatrix = mat2(cos(thetaPly), -sin(thetaPly), sin(thetaPly), cos(thetaPly));
        vec2 plyCenter = yarnCenter + vec2(cos(thetaPly), sin(thetaPly)) * 0.35 * plyRadius;
        vec2 diff = plyInverseMatrix * (uv - plyCenter) * scale;
        density += sampleFiberDensity(e, B, length(diff));
    }
    
    return density;
}


void main()
{
    vec2  uv = vScreenCoords * 4.0;
    // Output to screen
    float density = sampleYarnDensity(uv, uPlyCount, uPlyRadius, uPlyAngle,
                                      uEN, uEB, uDensityE, uDensityB);
    FragColor = vec4(density, density, density, 1.0);
}
