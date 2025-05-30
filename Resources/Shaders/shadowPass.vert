#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 inPosition;

void main()
{
	//gl_Position = lightSpaceMatrix * vec4(inPosition, 1.0);
}