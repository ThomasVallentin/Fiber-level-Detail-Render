#version 460 core

layout(isolines, equal_spacing) in;


void main() {
    int verticesCount = gl_PatchVerticesIn;
    float u = gl_TessCoord.x;
    int iu = int(u * (verticesCount - 1));
    float diff = fract( u * (verticesCount - 1));
    float v = gl_TessCoord.y;
	vec4 p;
	if (iu != verticesCount - 1)
        p = mix( gl_in[iu].gl_Position, gl_in[iu+1].gl_Position, diff);
	else
		p = gl_in[iu].gl_Position;
    
    gl_Position = p;
}