#version 430 core

smooth in vec3 eyeNormal;           // Vertex normal in eye space
smooth in vec3 directionToLightEye; // Direction to light in eye space

out vec4 fragColour;             // Outgoing fragment colour

void main()
{
    // Light colours
    vec3 ambientLightColour = vec3(0.1, 0.1, 0.1);
    vec3 diffuseLightColour = vec3(0.9, 0.5, 0.5);
    vec3 specularLightColour = vec3(1.0, 1.0, 1.0);

    // Material colours
    vec3 ambientMaterialColour  = vec3(1.0);
    vec3 diffuseMaterialColour  = vec3(1.0);
    vec3 specularMaterialColour = vec3(1.0);
    float specularPower = 64.0f;

	// Add ambient contribution
	fragColour = vec4(ambientLightColour * ambientMaterialColour, 1.0);

	// Calculate the diffuse intensity by getting the dot product of the normal and the light direction
    float diffuseIntensity = max(0.0, dot(normalize(eyeNormal), normalize(directionToLightEye)));

	// If the fragment is facing the light source it will receive a diffuse lighting contribution
    if (diffuseIntensity > 0.0)
    {
        // Add in diffuse colour calculated as multiplication of diffuse intensity and diffuse colour
        fragColour += vec4(diffuseLightColour * diffuseMaterialColour * diffuseIntensity, 1.0);

        // Specular light
        vec3 specularReflectionDirection = normalize( reflect(normalize(-eyeNormal), normalize(directionToLightEye)) );
        float specularIntensity = max(0.0, dot( normalize(eyeNormal), specularReflectionDirection) );

        // If the diffuse light is more than zero then (and only then) calculate specular contribution (pow function is expensive!)
        if (diffuseIntensity > 0.0)
        {
            float specularPower = pow(specularIntensity, specularPower);
            fragColour.rgb += specularLightColour * specularMaterialColour * specularPower;
        }
    }
}
