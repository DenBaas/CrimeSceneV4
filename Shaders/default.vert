//Author: Bas Rops - ??-??-2014
//Last edit: Bas Rops - 09-06-2014
#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

uniform mat4 viewProjectionMatrix;
uniform mat4 meshModelMatrix;

out vec3 normal;
out vec2 texcoord;

void main()
{
	texcoord = a_texcoord;
	normal = normalize((transpose(inverse(meshModelMatrix)) * vec4(a_normal, 1.0)).xyz);

	gl_Position = viewProjectionMatrix * meshModelMatrix * vec4(a_position, 1.0);
}