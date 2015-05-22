#version 400

attribute vec3 a_position;
uniform mat4 modelViewProjectionMatrix;
varying vec2 texCoord;

void main()
{
	texCoord = (vec2(a_position) + vec2(1, 1)) / 2.0;	
	gl_Position = modelViewProjectionMatrix * vec4(a_position,1);
}