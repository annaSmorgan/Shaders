#version 420
//the layout command tells the shader the format of the points 
//being passed into the shader, in this scenario all vertices
//will be treated as individual points and not as triangle geometry
layout(points) in;
//we will output triangle strip data from this shader with 4 verts
//emited for every one vert coming in.

layout(points, max_vertices = 32) out;

//accepting input from the previous stage of the pipeline as a structure
in Vertex
{
	float pType;
	vec4 pPos;
	vec4 pVelocity;
	float pAge;
}vertex[];

//outputting to transform feedback as individual native components
//no fragment shader attached!
out float Type;
out vec4 Position;
out vec4 Velocity;
out float Age;

//uniforms
uniform float deltaTime;
uniform float totalTime;

//texture used for random values
uniform sampler2D randomTexture;

//atomic buffer binding
layout(binding = 0, offset = 0) uniform atomic_uint particleCount;


//Definitions of avaliable particle types
#define PTYPE_EMITTER 0.0
#define PTYPE_PARTICLE 1.0


vec4 GetRandomVelocity(float fTime)
{
	vec3 velocity = (texture(randomTexture, vec2(fTime * 5.0, fTime * 5.0)).rbg - 0.5) * 2.0;
	return vec4(velocity, 0.0);

}

void main( )
{
	
	
	if (vertex[0].pType == PTYPE_EMITTER) //This is the particle emitter, let it emit particles
	{
		for (int i = 0; i < 1; ++i) //loop and emit 30 particles each frame from this emitter.
		{
			Type = PTYPE_PARTICLE;
			Position = vertex[0].pPos;
			Velocity = GetRandomVelocity(totalTime * (i+1));
			Age = 0.0;
			EmitVertex();
			EndPrimitive();
			atomicCounterIncrement(particleCount);
		}
		Type = vertex[0].pType;
		Position = vec4(-0.5f, -9.f, 5.f, 1.f) + (sin(totalTime) * vertex[0].pVelocity);
		Velocity = vertex[0].pVelocity;
		Age = vertex[0].pAge;
		EmitVertex();
		EndPrimitive();
		atomicCounterIncrement(particleCount);
	}
	else if (vertex[0].pType == PTYPE_PARTICLE)
	{	
		float age = vertex[0].pAge + deltaTime;
		if (age <= 2.0)
		{
			Type = vertex[0].pType;
			Position = vec4(vertex[0].pPos.xyz + vertex[0].pVelocity.xyz * deltaTime, 1.0);
			Velocity = vertex[0].pVelocity;
			Age = age;

			EmitVertex();
			EndPrimitive();
			atomicCounterIncrement(particleCount);
		}
	}	

}