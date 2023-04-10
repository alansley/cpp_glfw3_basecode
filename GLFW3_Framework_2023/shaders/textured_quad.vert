#version 430 core

in vec3 position;
in vec2 texCoords;

out vec2 interpolatedTexCoords;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

void main()
{
    // Outgoing to fragment shader
    interpolatedTexCoords = texCoords;

	// Project our geometry
    gl_Position = projectionMatrix * modelMatrix * vec4(position.xyz, 1.0);
}
