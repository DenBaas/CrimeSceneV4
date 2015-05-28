//Author: Bas Rops - ??-??-2014
//Last edit: Bas Rops - 09-06-2014
#version 330

layout(location = 0) in vec3 a_position;

uniform mat4 viewProjectionMatrix;

out vec3 texcoord;

void main()
{
	vec3 pos = a_position;
	texcoord = pos;
	gl_Position = viewProjectionMatrix * vec4(pos,1);
}