//Author: Bas Rops - ??-??-2014
//Last edit: Bas Rops - 09-06-2014
#version 330

uniform sampler2D s_texture;

uniform bool useTexture;
uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matShinyness;

uniform vec3 wandPosition;
uniform vec3 wandDirection;
uniform bool objectVisible;
uniform float maxLength;
uniform float spotCutoff;

in vec2 texcoord;
in vec4 fragPosition;
in vec3 lightVertexDirection;

layout(location = 0) out vec4 fragColor;

void main()
{
    //Calculate the angle between wandDirection and lightVertexDirection
    float angle = acos(dot(normalize(wandDirection), normalize(lightVertexDirection)));

	//If the fragment is within the spotlight
	if(angle < radians(spotCutoff) && length(fragPosition - vec4(wandPosition,1)) < maxLength)
	{
		if(useTexture)
		{
			//Get the fragColor from the texture
			fragColor = texture2D(s_texture, texcoord);

			if(!objectVisible)
				//Make the fragColor blue. Keep using red and green channels so that the color is not the same everywhere, but divide them by 2. Also keep using the alpha value of the texture.
				fragColor = vec4(0.5, 0.5, 1.0, fragColor.w);
			else
				//If the object is standard visible, divide red and green channels by 5, so that the color goes to a darker blue
				fragColor = vec4(fragColor.x / 5.0, fragColor.y / 5.0, 1.0, fragColor.w);
		}
		else
		{
			if(!objectVisible)
				fragColor = vec4(0.5, 0.5, 1.0, 1.0);
			else
				fragColor = vec4(matDiffuse.x / 5.0, matDiffuse.y / 5.0, 1.0, 1.0);
		}
	}
	else
	{
		//Object is not within the spotlight and is set to being not visible, so discard the fragment
		if(!objectVisible)
			discard;

		//Object is not within the spotlight, but still visible without polylight, so multiply the fragColor of the texture with a dark (near black) color
		else
		{
			if(useTexture)
				fragColor = texture2D(s_texture, texcoord) * vec4(0.1, 0.1, 0.1, 1.0);
			else
				fragColor = vec4(matDiffuse, 1.0) * vec4(0.1, 0.1, 0.1, 1.0);
		}
	}
}