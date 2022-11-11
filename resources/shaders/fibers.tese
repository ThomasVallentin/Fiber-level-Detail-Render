#version 460 core

layout( isolines, equal_spacing ) in;

#define PI 3.14159265359

struct FiberParameters {
    float plyRadius;
    int plyCount;
    float plyRotSpeed;
    float Rmin;
    float Rmax;
    float fibersRotSpeed;
};

uniform FiberParameters fiberParams = FiberParameters(0.2,  // ply radius
                                                      3,    // ply count
                                                      2.0,  // ply rot speed
                                                      0.1,  // Rmin
                                                      1.0,  // Rmax
                                                      4.0   // fibers rot speed
                                                      ); // Should be an uniform block instead

vec3 cubicBezier(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    float oneMinT = (1 - t);
    float a = oneMinT * oneMinT * oneMinT;
    float b = 3.0 * oneMinT * oneMinT * t;
    float c = 3.0 * oneMinT * t * t;
    float d = t * t * t;

    return a * p0 + b * p1 + c * p2 + d * p3; 
}

vec3 cubicBezierDerivative(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    float oneMinT = (1 - t);
    float a = 3.0 * oneMinT * oneMinT;
    float b = 6.0 * oneMinT * t;
    float c = 3.0 * t * t;

    return a * (p1 - p0) + b * (p2 - p1) + c * (p3 - p2); 
}

float R(float Ri, float Rmin, float Rmax, float thetaI, float theta, float rotLength) {
    return Ri * 0.5 * (Rmin + Rmax + (Rmax - Rmin) * cos(thetaI + rotLength * theta));
} 


float initialPlyAngle(int plyCount, int plyIndex) {
    return 2.0 * PI * plyIndex / plyCount;
}


vec3 yarnToPly(vec3 yarnTCrossN, vec3 yarnNormal, float plyRadius, 
               float thetaPly, float theta) {
    return 0.5 * plyRadius * ((cos(thetaPly + theta) * yarnNormal) +
                              (sin(thetaPly + theta) * yarnTCrossN));
}


vec3 plyToFiber(vec3 plyTCrossN, vec3 plyNormal, float fiberRadius,
                float thetaI, float theta, float elipseN, float elipseB) {
    return fiberRadius * (cos(thetaI + theta) * plyNormal * elipseN +
                          sin(thetaI + theta) * plyTCrossN * elipseB);
}


void main() {
    int verticesPerFiber = int(gl_TessLevelOuter[1]);
    int fibersCount = int(gl_TessLevelOuter[0]);
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    int vtxIndex = int(u * (verticesPerFiber - 1));
    int fiberVtxIndex = vtxIndex % verticesPerFiber;
    int fiberIndex = int(v * (fibersCount - 1));

    int plyIndex = fiberIndex % fiberParams.plyCount;  

    vec3 yarnCenter = cubicBezier(gl_in[0].gl_Position.xyz,
                                  gl_in[1].gl_Position.xyz,
                                  gl_in[2].gl_Position.xyz,
                                  gl_in[3].gl_Position.xyz,
                                  u);
    vec3 yarnTangent = normalize(cubicBezierDerivative(gl_in[0].gl_Position.xyz,
                                             gl_in[1].gl_Position.xyz,
                                             gl_in[2].gl_Position.xyz,
                                             gl_in[3].gl_Position.xyz,
                                             u));
    vec3 yarnNormal = vec3(0.0, 1.0, 0.0); // To read from the buffers
    vec3 yarnTCrossN = cross(yarnTangent, yarnNormal);
    yarnNormal = cross(yarnTangent, yarnTCrossN);
    float thetaPly = initialPlyAngle(fiberParams.plyCount, plyIndex);

    vec3 deltaPly = yarnToPly(yarnTCrossN,
                              yarnNormal, 
                              fiberParams.plyRadius, 
                              thetaPly, 
                              u * fiberParams.plyRotSpeed);

    vec3 plyNormal = normalize(deltaPly);
    vec3 plyTCrossN = cross(yarnTangent, plyNormal);
    float fiberRadius = R(0.2,     // Should be different for each fiber
                          fiberParams.Rmin,    
                          fiberParams.Rmax,    
                          fiberIndex,  // Just a placeholder, should not be equal to that
                          0.1,       // Just a placeholder, should not be equal to that
                          u * fiberParams.fibersRotSpeed);

    vec3 deltaFiber = plyToFiber(plyTCrossN, 
                                 plyNormal, 
                                 fiberRadius,
                                 cos(v), // Just a placeholder, should not be equal to that
                                 0.1,      // Just a placeholder, should not be equal to that
                                 1.0, 1.0);     


    // vec3 fiberCenter = yarnCenter + deltaPly + deltaFiber;

    // gl_Position = vec4(fiberCenter, 1.0);
    gl_Position = vec4(yarnCenter + deltaPly + deltaFiber, 1.0);
}
