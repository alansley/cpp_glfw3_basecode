#version 430 core

in vec2 interpolatedTexCoords;

out vec4 colour;

uniform sampler2D textureMap;

void main()
{
	colour = texture(textureMap, interpolatedTexCoords);
	
	// If you're getting alpha problems uncomment the below for some visual debugging.
	// Red = alpha values exist and are below 1.0, Blue = alpha values probably don't exist / or are all precisely 1.0
	//if (colour.a < 1) colour = vec4(1, 0, 0, 1); else colour = vec4(0, 0, 1, 1);
}
