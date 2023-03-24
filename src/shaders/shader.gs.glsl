#version 410 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;


in TS_OUT {
    int globalFiberIndex;
} gs_in[]; 

void main() {    
    float thickness = 1.5f;

    if (gs_in[0].globalFiberIndex % 22 == 0) 
        thickness = 10.f;

    vec2 u_viewportInvSize = vec2(0.0006f,0.0008f);

    vec4 p1 = gl_in[0].gl_Position;
    vec4 p2 = gl_in[1].gl_Position;

    vec2 dir = normalize(p2.xy - p1.xy);
    vec2 normal = vec2(dir.y, -dir.x);

    vec4 offset1, offset2;
    offset1 = vec4(normal * u_viewportInvSize * (thickness * p1.w), 0, 0);
    offset2 = vec4(normal * u_viewportInvSize * (thickness * p2.w), 0, 0); // changing this to p2 fixes some of the issues

    vec4 coords[4];
    coords[0] = p1 + offset1;
    coords[1] = p1 - offset1;
    coords[2] = p2 + offset2;
    coords[3] = p2 - offset2;

    for (int i = 0; i < 4; ++i) {
        gl_Position = coords[i];
        EmitVertex();
    }
    EndPrimitive();
}  