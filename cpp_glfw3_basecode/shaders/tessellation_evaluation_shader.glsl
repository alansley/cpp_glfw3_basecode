#version 430 core

layout (quads, fractional_odd_spacing) in;

uniform sampler2D tex_displacement;

uniform mat4 modelMVP;
uniform float dmap_depth;

in TCS_OUT
{
	vec2 tc;
} tes_in[];


out TES_OUT
{
	vec2 tc;
} tes_out;


void main(void)
{
	vec2 tc1 = mix(tes_in[0].tc, tes_in[1].tc, gl_TessCoord.x);
	vec2 tc2 = mix(tes_in[2].tc, tes_in[3].tc, gl_TessCoord.x);
	vec2 tc = mix(tc2, tc1, gl_TessCoord.y);

	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 p = mix(p2, p1, gl_TessCoord.y);

	p.y += texture(tex_displacement, tc).r * dmap_depth;

	// DO NOT do the below to scale the landscape, because the TessLevel is based on ORIG size, not this expanded size
	//double d = 20.0;
	//gl_Position = mvp * vec4(p.x * d, p.y - 195, p.z * d, p.w);

	gl_Position = modelMVP * p;

	tes_out.tc = tc;
}
