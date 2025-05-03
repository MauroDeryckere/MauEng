#version 450

// Simple full screen triangle
vec2 gPositions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2(3.0, -1.0),
        vec2(-1.0, 3.0)
    );

void main() 
{
    gl_Position = vec4(gPositions[gl_VertexIndex], 0.0, 1.0);
}