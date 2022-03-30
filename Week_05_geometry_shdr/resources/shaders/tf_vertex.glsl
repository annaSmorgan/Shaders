#version 420

in float particleType;
in vec4 particlePos;
in vec4 particleVelocity;
in float particleAge;

//We can output a structure from a shader as well
//we do not need to only output basic native types
out Vertex
{
	float pType;
	vec4 pPos;
	vec4 pVelocity;
	float pAge;

} vertex;

//This is a simple pass through shader that simply sends the particle data into the next stage of the pipeline
//There is not model or view space transform carried out here either
void main() 
{ 
	vertex.pType		= particleType;
	vertex.pPos			= particlePos;
	vertex.pVelocity	= particleVelocity;
	vertex.pAge			= particleAge;

	
	
}