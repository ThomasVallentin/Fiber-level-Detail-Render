// tessellation evaluation shader
#version 410 core

// == Inputs ==

layout (isolines, equal_spacing) in;

patch in vec4 pPrevPoint;
patch in vec4 pNextPoint;


// == Uniforms

uniform mat4 model;           // the model matrix
uniform mat4 view;            // the view matrix


// == Outputs ==

out TS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} ts_out; 


vec3 catmullCurve(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4, float u) {
    float u2 = u * u; 
    float u3 = u2 * u; 

	float b0 = -u + 2.0 * u2 - u3;
	float b1 = 2.0 + -5.0 * u2 + 3.0 * u3;
	float b2 = u + 4.0 * u2 + - 3.0 * u3;
	float b3 = -1.0 * u2 + u3;
    return 0.5 * (b0 * pos1 + b1 * pos2 + b2 * pos3 + b3 * pos4);
}

vec3 catmullDerivative(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u) {
    float u2 = u * u; 

    float b0 = -1.0 + 4.0 * u - 3.0 * u2;
    float b1 = -10.0 * u + 9.0 * u2;
    float b2 = 1.0 + 8.0 * u - 9.0 * u2;
    float b3 = -2.0 * u + 3.0 * u2;
    return 0.5 * (b0 * pos1 + b1 * pos2 + b2 * pos3 + b3 * pos4);
}

vec3 catmullSecondDerivative(vec3 pos1, vec3 pos2,vec3 pos3, vec3 pos4,float u) {
    float b0 = 4.0 - 6.0 * u;
    float b1 = -10.0 + 18.0 * u;
    float b2 = 8.0 - 18.0 * u;
    float b3 = -4.0 + 6.0 * u;
    return 0.5 * (b0 * pos1 + b1 * pos2 + b2 * pos3 + b3 * pos4);
}


void main() {
    float u = gl_TessCoord.x;

    vec3 cp1 = pPrevPoint.xyz;
    vec3 cp2 = gl_in[0].gl_Position.xyz;
    vec3 cp3 = gl_in[1].gl_Position.xyz;
    vec3 cp4 = pNextPoint.xyz;
    
    // Yarn center using a catmull rom interpolation of the control points
    vec3 curvePoint = catmullCurve(cp1, cp2, cp3, cp4, u);

    vec3 tangent = normalize(catmullDerivative(cp1, cp2, cp3, cp4, u));
    vec3 normal = normalize(catmullSecondDerivative(cp1, cp2, cp3, cp4, u));
    vec3 bitangent = normalize(cross(normal, tangent));
    normal = normalize(cross(bitangent, tangent));
    
    // Outputs
    gl_Position      =      view * model * vec4(curvePoint, 1.0);
    ts_out.normal    = vec3(view * model * vec4(normal, 0.0));
    ts_out.tangent   = vec3(view * model * vec4(tangent, 0.0));
    ts_out.bitangent = vec3(view * model * vec4(bitangent, 0.0));
}
