#version 450

layout(push_constant) uniform PushConstants {
	mat4 view;
	mat4 proj;
} pc;

const vec3 gVertexPositions[36] = vec3[]
(
	// +X
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(1, -1, 1),
	vec3(1, 1, -1), vec3(1, -1, 1), vec3(1, 1, 1),

	// -X
	vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(-1, 1, 1),
	vec3(-1, -1, -1), vec3(-1, 1, 1), vec3(-1, -1, 1),

	// +Y
	vec3(-1, 1, -1), vec3(1, 1, -1), vec3(1, 1, 1),
	vec3(-1, 1, -1), vec3(1, 1, 1), vec3(-1, 1, 1),

	// -Y
	vec3(-1, -1, 1), vec3(1, -1, 1), vec3(1, -1, -1),
	vec3(-1, -1, 1), vec3(1, -1, -1), vec3(-1, -1, -1),

	// +Z
	vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
	vec3(1, -1, 1), vec3(-1, 1, 1), vec3(1, 1, 1),

	// -Z
	vec3(-1, -1, -1), vec3(1, -1, -1), vec3(1, 1, -1),
	vec3(-1, -1, -1), vec3(1, 1, -1), vec3(-1, 1, -1)
);

// Out
layout(location = 0) out vec3 outFragLocalPos;

void main()
{
	vec3 pos = gVertexPositions[gl_VertexIndex];
	outFragLocalPos = pos;
	gl_Position = pc.proj * pc.view * vec4(pos, 1.0f);
}