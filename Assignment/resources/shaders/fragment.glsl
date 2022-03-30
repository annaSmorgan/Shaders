#version 150

in Vertex //data about the vertex from vertex shader
{
	vec4 Pos;
	vec4 Normal;
	vec4 Colour; 
	vec2 UV;
	vec4 ShadowCoord;
    vec3 eyeDir;
}vertex;

out vec4 FragColor;

uniform sampler2D DiffuseTexture;
uniform sampler2D ShadowTexture;
uniform mat4 Model;
 
//variables for each light type
struct Light
{
	vec3  colour;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct DirLight 
{
	Light lights;
    vec4 direction;
};  

struct PointLight {  
	Light lights;
    vec4 position;
    
    float constant;
    float linear;
    float quadratic;  
};  

struct SpotLight { 
	PointLight plights;
    vec4 direction;
    float angle;
}; 

const Light sceneLight = Light(vec3(0.335f, 0.255f, 0.155f), vec3( 0.125, 0.125,0.125), vec3(1.0,1.0,1.0), vec3(0.7,0.7,0.7));

//light uniforms
uniform DirLight dirLight;
uniform SpotLight pointLight;
uniform SpotLight spotLight;

//shadows
const float SHADOW_BIAS = 0.005f;
const vec2 poissonDisk[4] = vec2[](
	vec2( -0.94201624, -0.39906216),
	vec2( -0.94558609, -0.76890725),
	vec2( -0.094184101, -0.92938870),
	vec2( -0.34495938, -0.29387760)
);

vec3 CalcDirLight(DirLight light)
{
	//direction of light
	vec3 lightDirection = normalize(light.direction.xyz);

	//shadows
	float shadowFactor = 1.0;

	float cosT = dot(normalize(vertex.Normal.xyz), normalize(lightDirection));
	cosT = clamp(cosT, 0, 1);

	float SBias = SHADOW_BIAS * tan(acos(cosT));
	SBias = clamp(SBias, 0, 0.01);

	//shadows
	for (int i = 0; i < 4; i++)
	{
		if (texture(ShadowTexture, vertex.ShadowCoord.xy + poissonDisk[i]/700.0).z < vertex.ShadowCoord.z - SBias)
		{
			shadowFactor -= 0.2;
		}
	}

	vec3 albedo = texture(DiffuseTexture, vertex.UV).xyz;
	//Phong ambient color
	vec3 ambient = sceneLight.ambient * light.lights.ambient * light.lights.colour;
	//Phong Diffuse
	vec3 diffuse = max(0,dot(vertex.Normal.xyz, lightDirection)) * sceneLight.diffuse * light.lights.diffuse * light.lights.colour;
	
	//Calculate Specular Component
    vec3 HalfDir = normalize(-vertex.eyeDir + lightDirection);
	vec3 specular = pow(max(0,dot(HalfDir, vertex.Normal.xyz)),32) * sceneLight.specular * light.lights.specular * light.lights.colour;

    return (albedo * ambient + albedo * (diffuse + specular) * shadowFactor);
}  

vec3 CalcPointLight(PointLight light)
{
	//direction of light
	vec3 lightDirection = normalize(vertex.Pos.xyz - (Model * light.position).xyz);

	vec3 albedo = texture(DiffuseTexture, vertex.UV).xyz;
	//Phong ambient color
	vec3 ambient = sceneLight.ambient * light.lights.ambient * light.lights.colour;
	//Phong Diffuse
	vec3 diffuse = max(0,dot(vertex.Normal.xyz, lightDirection))* sceneLight.diffuse * light.lights.diffuse * light.lights.colour;
	
	//Specular 
	vec3 reflectDir = reflect(-lightDirection, normalize(vertex.Normal).xyz);
    vec3  specular = pow(max(dot(normalize(vertex.eyeDir - vertex.Pos.xyz), reflectDir), 0.0), 0.0)* sceneLight.diffuse * light.lights.specular * light.lights.colour;

	//calc bleed off
	float distance = length((Model * light.position).xyz - vertex.Pos.xyz);
    float bleedOff = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 

    ambient  *= bleedOff;
    diffuse  *= bleedOff;
    specular *= bleedOff;

    return (albedo * ambient + albedo * (diffuse + specular));
} 

vec3 CalcSpotLight(SpotLight light)
{
	vec3 dirToVertex = normalize(vertex.Pos.xyz - (Model * light.plights.position).xyz);
	float spotFac = dot(dirToVertex, normalize((light.direction)).xyz);

	//if direction is within angle 
	if (spotFac > light.angle)
	{
		//use point light to calc then ristrict by angle
		vec3 result = CalcPointLight(light.plights);
		float intensity = (1.0 - (1.0 - spotFac)/(1.0 - light.angle));
		return result * intensity;
	}
	else
	{
		return vec3(0.0f, 0.0f, 0.0f); //no light
	}
} 

void main()
{

	//add up all lights values
	vec3 result = vec3(0.0f);
    //Directional lighting
    result += CalcDirLight(dirLight);
    //Point lights
	result += CalcSpotLight(pointLight);
    //Spot light
	result += CalcSpotLight(spotLight);
	
	//gamma correction
	vec3 gammaCorrected = pow(result, vec3(1.0/2.2));
	FragColor = vec4(gammaCorrected, 1.0f);
}