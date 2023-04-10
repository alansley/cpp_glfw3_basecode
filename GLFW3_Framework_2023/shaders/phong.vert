#version 430 core

// --- Incoming per-vertex data ---
in vec3 vertexPosition;
in vec3 vertexNormal;

// --- Outgoing (to the fragment shader) per-vertex data ---
smooth out vec3 eyeNormal;        // Vertex normal in eye space
smooth out vec3 directionToLightEye; // Direction to light in eye space

uniform mat4 projectionMatrix;  // Eye->Screen (i.e. rasterisation)
uniform mat4 viewMatrix;        // World->Eye
uniform mat4 modelMatrix;       // Model->World
uniform mat3 normalMatrix;      // Normal matrix. Note: The normal matrix is just a 3x3 as that's all that's req'd.

//uniform float time;

/*
// Gold Noise ©2015 dcerisano@standard3d.com
//  - based on the Golden Ratio, PI and Square Root of Two
//  - superior distribution
//  - fastest noise generator function
//  - works with all chipsets (including low precision)

precision lowp float;

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed){
    return fract(tan(distance(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
}
*/

void main()
{
    // Calculate the vertex normal in eye space
    eyeNormal = normalize(normalMatrix * vertexNormal);

	// Get vertex position in eye coordinates
	vec4 eyePosition4 = viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
	vec3 eyePosition3 = eyePosition4.xyz / eyePosition4.z; // Normalise

	vec3 lightPosition = vec3(0.0, 0.0, 1000.0);
	directionToLightEye = normalize(lightPosition - eyePosition3);

	// Project our geometry. Note: Matrix multiplication is not commutative so the order of multiplication matters!
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);

	//float temp = vertexLocation.x + (gold_noise(vertexPosition.xy, time) * (sin(time / 2.0f) * 20.0f) );

    // Project our geometry
	//gl_Position = pMatrix * vMatrix * mMatrix * vec4(temp, vertexPosition.yz, 1.0);
}
