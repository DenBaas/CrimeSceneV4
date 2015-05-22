//Author: Bas Rops - ??-??-2014
//Last edit: Bas Rops - 09-06-2014
#version 330

uniform samplerCube s_cubemap;

in vec3 texcoord;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = texture(s_cubemap, texcoord);
}