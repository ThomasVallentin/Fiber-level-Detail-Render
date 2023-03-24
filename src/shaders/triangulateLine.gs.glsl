#version 460 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

const float width = 0.1;
const vec3 normal = vec3(0, 1.0, 0);

uniform mat4 uProjMatrix;


void main()
{  
    vec3 pntA = gl_in[0].gl_Position.xyz;
    vec3 pntB = gl_in[1].gl_Position.xyz;

    vec3 toCamera = normalize(-pntA);
    vec3 right = cross(toCamera, normal);
    vec3 lineSide = cross(toCamera, right) * 0.5;

    gl_Position = uProjMatrix * vec4(pntA - lineSide * width, 1.0);
    EmitVertex();
    gl_Position = uProjMatrix * vec4(pntA + lineSide * width, 1.0);
    EmitVertex();
    
    gl_Position = uProjMatrix * vec4(pntB - lineSide * width, 1.0);
    EmitVertex();
    gl_Position = uProjMatrix * vec4(pntB + lineSide * width, 1.0);
    EmitVertex();
}