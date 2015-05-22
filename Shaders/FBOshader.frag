#version 400

varying vec2 texCoord;
uniform sampler2D frame_texture;
uniform sampler2D info_texture;

void main()
{
    vec4 color = texture(info_texture, texCoord);	
	gl_FragColor = color;
}