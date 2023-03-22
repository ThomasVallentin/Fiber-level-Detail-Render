#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelMatrix = mat4(1.0);
uniform mat4 uViewMatrix = mat4(1.0);

out VertexData
{
    vec3 position;
    vec3 normal;
    vec2 texCoord;
} outVertex;


void main() 
{
    vec4 position = uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
    outVertex.position = position.xyz;
    outVertex.normal = (uViewMatrix * uModelMatrix * vec4(aNormal, 1.0)).xyz;
    outVertex.texCoord = aTexCoord;

    gl_Position = position;
}
