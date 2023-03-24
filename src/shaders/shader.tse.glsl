// tessellation evaluation shader
#version 410 core

layout (isolines, equal_spacing) in;

float PI = 3.14159;

uniform mat4 model;           // the model matrix
uniform mat4 view;            // the view matrix
uniform mat4 projection;      // the projection matrix

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
} ts_out; 

vec3 bezierCurve(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u){
    float b0 = (1.-u) * (1.-u) * (1.-u);
    float b1 = 3. * u * (1.-u) * (1.-u);
    float b2 = 3. * u * u * (1.-u);
    float b3 = u * u * u;
    return b0*pos1 + b1*pos2 + b2*pos3 + b3*pos4;
}

vec3 bezierDerivative(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u) {
    float b0 = -3 * (1.-u) * (1.-u);
    float b1 = (1 - u) * (1 - 3*u);
    float b2 = 3*u*(2 - 3*u);
    float b3 = 3 * u * u;
    return b0*pos1 + b1*pos2 + b2*pos3 + b3*pos4;  
}

vec3 catmullCurve(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u){
	float b0 = (-1.f * u) + (2.f * u * u) + (-1.f * u * u * u);
	float b1 = (2.f) + (-5.f * u * u) + (3.f * u * u * u);
	float b2 = (u) + (4.f * u * u) + (-3.f * u * u * u);
	float b3 = (-1.f * u * u) + (u * u * u);
    return 0.5f * (b0*pos1 + b1*pos2 + b2*pos3 + b3*pos4);
}

vec3 catmullDerivative(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u) {
    float b0 = -1.f + 4.f*u -3.f*u*u;
    float b1 = -10.f*u + 9.f*u*u;
    float b2 = 1.f + 8.f*u -9.f*u*u;
    float b3 = -2.f * u + 3.f * u*u;
    return 0.5f * (b0*pos1 + b1*pos2 + b2*pos3 + b3*pos4);  
}

void main() {
    int n_ply = 3;

    int n_fiber = int(gl_TessLevelOuter[0]);
    int n_fiber_per_ply = int(gl_TessLevelOuter[0])/ n_ply;

    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    int fiberIndex = int(v * n_fiber_per_ply);
    int plyIndex = int(v * n_fiber) % n_ply;

    vec3 pc1 = p_1.xyz;
    vec3 pc2 = gl_in[1].gl_Position.xyz;
    vec3 pc3 = gl_in[2].gl_Position.xyz;
    vec3 pc4 = p2.xyz;

    //vec3 yarnCenter = bezierCurve(pc1,pc2,pc3,pc4,u); 
    vec3 yarnCenter = catmullCurve(pc1,pc2,pc3,pc4,u);

    vec3 N_yarn = vec3(0.0, 1.0, 0.0);
    //vec3 T_yarn = normalize(bezierDerivative(pc1,pc2,pc3,pc4,u));
    vec3 T_yarn = normalize(catmullDerivative(pc1,pc2,pc3,pc4,u));
    vec3 B_yarn = cross(T_yarn,N_yarn); 
   
    float thetaPly = 2 * PI * plyIndex / n_ply;
    vec3 displacement_ply = 0.5 * R_ply * (cos(thetaPly + u * theta) * N_yarn + (sin(thetaPly + u * theta) * B_yarn));

    //float Ri = R[fiberIndex];
    float thetaI = 2.0f * PI * fiberIndex / n_fiber_per_ply;
    float Ri = R[fiberIndex % 4];
    float R_fiber = 0.5f * Ri * (Rmax + Rmin + (Rmax - Rmin)*cos(thetaI + s * u * theta));

    vec3 N_ply = (1. / length(displacement_ply) ) * displacement_ply;
    vec3 B_ply = cross(T_yarn,N_ply);
    vec3 displacement_fiber = R_fiber * (cos(thetaI + u * 2.0f * theta) * N_ply * eN + sin(thetaI +  u * 2.0f* theta) * B_ply * eB);

    gl_Position = projection * view * model * vec4(yarnCenter + displacement_ply + displacement_fiber , 1.0);
    ts_out.globalFiberIndex = int(v * n_fiber);
}