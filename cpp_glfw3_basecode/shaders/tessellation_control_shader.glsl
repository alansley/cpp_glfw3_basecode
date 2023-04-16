#version 430 core

layout (vertices = 4) out;

in VS_OUT
{
	vec2 tc;
} tcs_in[];

out TCS_OUT
{
	vec2 tc;
} tcs_out[];

uniform mat4 modelMVP;
uniform mat4 modelViewMatrix;

/*
float GetTessLevel(float Distance0, float Distance1)
{
    float AvgDistance = (Distance0 + Distance1) / 2.0;

    if (AvgDistance <= 2.0) {
        return 10.0;
    }
    else if (AvgDistance <= 5.0) {
        return 7.0;
    }
    else  // Implied > 5.0
    {
        return 3.0;
    }
}
*/

void main(void)
{
    /* For a clear example use something like min = 5 and max = 40 */

    const float MinDepth = 5.0;  // Minimum distance from camera - where MaxTessLevel will be used i.e. distance at which we should use maximum tessellation
    const float MaxDepth = 50.0; // Maximum distance from camera - where MinTessLevel will be used i.e. distance beyond which we should use minimum tesselation

    const float MinTessLevel = 1.0; // Minimum desired tessellation level (i.e. at furthest distance to geometry)
    const float MaxTessLevel = 8.0; // Maximum desired tessellation level (i.e. at closest distance to geometry)

    // Vertex position in camera (i.e. 'eye') coordinates
    vec4 p = modelViewMatrix * gl_in[gl_InvocationID].gl_Position;

    // 'Distance' from camera scaled between 0 and 1
    float depth = clamp( (length(p) - MinDepth) / (MaxDepth - MinDepth), 0.0, 1.0);

    // Interpolate between min/max tess levels
    float tessLevel = mix(MaxTessLevel, MinTessLevel, depth);

    // Set outer and inner tessellation levels to this value
    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = tessLevel;
    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	tcs_out[gl_InvocationID].tc = tcs_in[gl_InvocationID].tc;
}
