//Author: Bas Rops - ??-??-2014
//Last edit: Bas Rops - 09-06-2014
#version 330

uniform sampler2D s_texture;

uniform bool objectVisible;
uniform bool useTexture;
uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matShinyness;
uniform float matTransparency;

in vec3 normal;
in vec2 texcoord;

layout(location = 0) out vec4 fragColor;

void main()
{
	if(!objectVisible)
		discard;

	float lighting = (normal * vec3(1.0, 0.0, 0.0)).x;

	if(useTexture)
	{
		vec4 texture = texture2D(s_texture, texcoord);

		if(texture.w < 0.1)
			discard;

		fragColor = vec4(texture.xyz * clamp(lighting + 0.5, 0.3, 1.0), texture.w);
	}
	else
	{
		//TODO use matAmbient, diffuse, specular and (float) shinyness
		if(matTransparency < 0.1)
			discard;

		fragColor = vec4(matDiffuse * clamp(lighting + 0.5, 0.3, 1.0), matTransparency);
	}
}