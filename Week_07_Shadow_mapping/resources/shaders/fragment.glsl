#version 150

in Vertex
{
	vec4 Pos;
	vec4 Normal;
	vec4 Colour; 
	vec2 UV;
	vec3 HalfDir;
	vec3 LightDir;
	vec4 ShadowCoord;
}vertex;

out vec4 FragColor;


uniform sampler2D DiffuseTexture;
uniform sampler2D ShadowTexture;
 
 struct Light{
 vec3 ambient;
 vec3 diffuse;
 vec3 specular;
 };

 const Light sceneLight = Light(vec3( 0.825, 0.125,0.025), vec3(1.0,1.0,1.0), vec3(0.9,0.7,0.4));
 const float specularTerm = 32.0;
 const float SHADOW_BIAS = 0.002;

void main()
{
	float shadowFactor = 1.0;

	//calculate shadow by testing depth
	if(texture(ShadowTexture, vertex.ShadowCoord.xy).z < vertex.ShadowCoord.z - SHADOW_BIAS)
	{
		shadowFactor = 0;
	}

	vec3 albedo = texture(DiffuseTexture, vertex.UV).xyz;

	//Phong ambient color
	vec3 ambient = sceneLight.ambient;
	//Phong Diffuse
	vec3 diffuse = max(0,dot(vertex.Normal.xyz, vertex.LightDir))* sceneLight.diffuse;
	
	//Calculate Specular Component
	vec3 specular = pow(max(0,dot(vertex.HalfDir, vertex.Normal.xyz)),specularTerm)*sceneLight.specular;
	
	vec3 linearColour = albedo * ambient + albedo * (diffuse + specular) * shadowFactor;
	//gamma correction
	vec3 gammaCorrected = pow(linearColour, vec3(1.0/2.2));
	FragColor = vec4(gammaCorrected, 1.0);

}
