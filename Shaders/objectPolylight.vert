//Author: Bas Rops - ??-??-2014
//Last edit: Bas Rops - 09-06-2014
#version 330

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 meshModelMatrix;
uniform vec3 wandPosition;
uniform float spotCutoff;

out vec2 texcoord;
out vec4 fragPosition;
out vec3 lightVertexDirection;
out vec3 normal;

void main()
{
	texcoord = a_texcoord;

	//fragPosition and lightVertexDirection use a_position, which needs to be translated and rotated by model- and meshMatrix
	fragPosition =   viewMatrix * meshModelMatrix * vec4(a_position, 1.0);
	//Calculate the vector from wandPosition to the vertex position, and set it as varying so it gets interpolated
    lightVertexDirection =  (viewMatrix *  meshModelMatrix * vec4(a_position, 1.0)).xyz - wandPosition;

	gl_Position = (projectionMatrix * viewMatrix)  * meshModelMatrix * vec4(a_position,1);
}