// tessellation evaluation shader
#version 410 core

layout (isolines, equal_spacing) in;

float PI = 3.14159;

uniform mat4 model;           // the model matrix
uniform mat4 view;            // the view matrix

uniform float R_ply; // R_ply
uniform float Rmin; 
uniform float Rmax; 
uniform float theta; // polar angle of the fiber helix
uniform float s; // length of rotation
uniform float eN; // ellipse scaling factor along Normal
uniform float eB; // ellipse scaling factor along Bitangent

//uniform float R[gl_TessLevelOuter[1]]; // where  R[i] contain the distance between fiber i and ply center
uniform float R[4];

patch in vec4 p_1;
patch in vec4 p2;


out TS_OUT {
    int globalFiberIndex;
    vec3 yarnCenter;
    vec3 fiberNormal;
    vec3 yarnTangent;
} ts_out; 


float sampleFiberDensity(float e, float B, float R)
{
    return R > 1.0 ? 0.0 : (1.0 - 2.0 * e) * pow((e - pow(e, R)) / (e - 1.0), B) + e;
}

vec3 bezierCurve(vec3 pos1, vec3 pos2, vec3 pos3, vec3 pos4, float u){
    float b0 = (1.-u) * (1.-u) * (1.-u);
    float b1 = 3. * u * (1.-u) * (1.-u);
    float b2 = 3. * u * u * (1.-u);
    float b3 = u * u * u;
    return b0 *pos1 + b1*pos2 + b2*pos3 + b3*pos4;
}

vec3 bezierDerivative(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u) {
    float b0 = -3 * (1.-u) * (1.-u);
    float b1 = (1 - u) * (1 - 3* u);
    float b2 = 3* u *(2 - 3* u);
    float b3 = 3 * u * u;
    return b0 *pos1 + b1*pos2 + b2*pos3 + b3*pos4;  
}

vec3 catmullCurve(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4, float u) {
    float u2 = u * u; 
    float u3 = u2 * u; 

	float b0 = -u + (2.0 * u2) - u3;
	float b1 = 2.0 + (-5.0 * u2) + (3.0 * u3);
	float b2 = u + (4.0 * u2) + (-3.0 * u3);
	float b3 = -1.0 * u2 + u3;
    return 0.5f * (b0 *pos1 + b1*pos2 + b2*pos3 + b3*pos4);
}

vec3 catmullDerivative(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u) {
    float b0 = -1.0 + 4.0 * u -3.0 * u * u;
    float b1 = -10.0 * u + 9.0 * u * u;
    float b2 = 1.0 + 8.0 * u -9.0 * u * u;
    float b3 = -2.0 * u + 3.0 * u * u;
    return 0.5f * (b0 * pos1 + b1*pos2 + b2*pos3 + b3*pos4);  
}

vec3 catmullSecondDerivative(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u) {
    float b0 = 4.0 - 6.0 * u;
    float b1 = -10.0 + 18.0 * u;
    float b2 = 8.0 - 18.0 * u;
    float b3 = -4.0 + 6.0 * u;
    return 0.5f * (b0 * pos1 + b1 * pos2 + b2 * pos3 + b3 * pos4);  
}

void main() {
    int plyCount = 3;

    int fiberCount = int(gl_TessLevelOuter[0]);
    int fibersPerPly = int(gl_TessLevelOuter[0]) / plyCount;

    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    int fiberIndex = int(v * (fiberCount + 1));
    int plyIndex = fiberIndex % plyCount;

    vec3 cp1 = p_1.xyz;
    vec3 cp2 = gl_in[0].gl_Position.xyz;
    vec3 cp3 = gl_in[1].gl_Position.xyz;
    vec3 cp4 = p2.xyz;
    
    // Yarn center using a catmull rom interpolation of the control points
    vec3 yarnCenter = catmullCurve(cp1, cp2, cp3, cp4, u);

    vec3 T_yarn = normalize(catmullDerivative(cp1, cp2, cp3, cp4, u));
    vec3 N_yarn = normalize(vec3(0.0,1.0,0.0)); 
    // vec3 N_yarn = normalize(catmullSecondDerivative(cp1, cp2, cp3, cp4, u)); 
    vec3 B_yarn = normalize(cross(N_yarn, T_yarn));
    N_yarn = cross(B_yarn, T_yarn);
    
    // Computing the ply offset from the yarn to the ply
    float globalU = gl_PrimitiveID + u;
    float thetaPly = 2 * PI * plyIndex / plyCount;
    vec3 displacement_ply = 0.5 * R_ply * (cos(thetaPly + globalU * theta) * N_yarn + (sin(thetaPly + globalU * theta) * B_yarn));

    float thetaI = 2.0 * PI * fiberIndex / fibersPerPly;
    float Ri = R[fiberIndex % 4];//fiberIndex < plyCount ? 0.0 : R[fiberIndex % 4];  // First fiber of each ply is the core fiber
    float R_fiber = 0.5 * Ri * (Rmax + Rmin + (Rmax - Rmin) * cos(thetaI + s * globalU * theta));

    vec3 N_ply = normalize(displacement_ply);
    vec3 B_ply = cross(T_yarn, N_ply);
    vec3 displacement_fiber = R_fiber * (cos(thetaI + globalU * 2.0 * theta) * N_ply * eN + sin(thetaI +  globalU * 2.0 * theta) * B_ply * eB);

    vec3 N_fiber = fiberIndex < plyCount ? N_ply : normalize(displacement_ply + displacement_fiber);
    vec3 T_fiber = T_yarn;
    vec3 B_fiber = normalize(cross(N_fiber, T_fiber));

    gl_Position = view * model * vec4(yarnCenter + displacement_ply + displacement_fiber, 1.0);
    ts_out.globalFiberIndex = int(v * fiberCount);
    ts_out.yarnCenter = vec3(view * model * vec4(yarnCenter, 1.0));
    ts_out.fiberNormal = vec3(view * model * vec4(N_fiber, 0.0));
    ts_out.yarnTangent = vec3(view * model * vec4(T_fiber, 0.0));
}
