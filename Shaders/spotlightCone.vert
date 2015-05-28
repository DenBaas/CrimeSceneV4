//Author: Bas Rops - ??-??-2014
//Last edit: Bas Rops - 09-06-2014
#version 330

layout(location = 0) in vec3 a_position;

uniform mat4 wandModelProjectionMatrix;
uniform vec3 wandPosition;

void main()
{
	gl_Position =  wandModelProjectionMatrix * vec4(a_position, 1);
}