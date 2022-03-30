#version 150

in Vertex
{
	vec4 Pos;
	vec4 Normal;
	vec4 Colour; 
	vec2 UV;
	vec3 HalfDir;
	vec3 LightDir;
}vertex;

out vec4 FragColor;


uniform sampler2D DiffuseTexture;
 
 struct Light{
 vec3 ambient;
 vec3 diffuse;
 vec3 specular;
 };

 const Light sceneLight = Light(vec3( 0.25, 0.25,0.25), vec3(1.0,1.0,1.0), vec3(1.0,1.0,1.0));
 const float specularTerm = 32.0;

void main()
{

	vec3 albedo = vec3( 1.0, 0.4, 0.4 );

	//Phong ambient color
	vec3 ambient = sceneLight.ambient;
	//Phong Diffuse
	vec3 diffuse = max(0,dot(vertex.Normal.xyz, vertex.LightDir))* sceneLight.diffuse;
	
	//Calculate Specular Component
	vec3 specular = pow(max(0,dot(vertex.HalfDir, vertex.Normal.xyz)),specularTerm)*sceneLight.specular;
	
	vec3 linearColour = albedo * (ambient + diffuse + specular); 
	//gamma correction
	vec3 gammaCorrected = pow(linearColour, vec3(1.0/2.2));
	FragColor = vec4(gammaCorrected, 1.0f);
}
