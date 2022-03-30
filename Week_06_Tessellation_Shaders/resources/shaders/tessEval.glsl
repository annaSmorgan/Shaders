#version 400

layout(triangles, equal_spacing, ccw) in;

in TessCont
{
	vec4 Normal;
	vec2 UV;
	vec4 eyeDir;
	vec4 lightDir;
}tessCS[];

out TessEval
{
	vec4 Normal;
	vec2 UV;
	vec4 eyeDir;
	vec4 lightDir;
}tessEval;

uniform sampler2D displacementTexture;
uniform float displacementScale;

void main()
{
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;
	vec3 p = gl_TessCoord.xyz;

	tessEval.Normal		= normalize(tessCS[0].Normal	* p.x + tessCS[1].Normal	* p.y + tessCS[2].Normal	* p.z);
	tessEval.UV			= tessCS[0].UV		* p.x + tessCS[1].UV		* p.y + tessCS[2].UV		* p.z;
	tessEval.eyeDir		= normalize(tessCS[0].eyeDir	* p.x + tessCS[1].eyeDir	* p.y + tessCS[2].eyeDir	* p.z);
	tessEval.lightDir	= normalize(tessCS[0].lightDir	* p.x + tessCS[1].lightDir	* p.y + tessCS[2].lightDir	* p.z);

	gl_Position = p0 * p.x + p1 * p.y + p2 * p.z;

	float dist = texture(displacementTexture, tessEval.UV).r;
	gl_Position.y += displacementScale * dist;
}