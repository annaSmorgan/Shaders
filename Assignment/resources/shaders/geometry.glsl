#version 150
//the layout command tells the shader the format of the points 
//being passed into the shader, in this scenario all vertices
//will be treated as individual points and not as triangle geometry
layout(points) in;
//we will output triangle strip data from this shader with 4 verts
//emited for every one vert coming in.

layout(triangle_strip, max_vertices = 4) out;

//accepting input from the previous stage of the pipeline as a structure
in Vertex
{
	vec4 Colour;
}vertex[];

//outputting to the fragment shader as individual native components
//why... no real reason should have stuck with the vertex structure
out vec4 vColour;
out vec2 vUV;

uniform mat4 ProjectionView; 
uniform vec4 cameraPosition;

void main( )
{
	//here we grab the position of our particle from gl_in and the stored gl_position 
	//which comes from the vertex shader
	vec3 P = gl_in[0].gl_Position.xyz;
	//the 0 value is that this is the first and only point passed to this shader, this shader operates
	//on all vertices emitted from the vertex shader, as we are dealing with points there is only one
	//hence the zero, if we were processing triangle data this could range from 0-2.
	//this could have been written as vec3 P = vertex[0].position; if we had sent the position
	//through as well as the colour information from the vertex shader. Just me showing you a 
	//different way of doing the same thing.
	
	//making the quads billboards means that they will always face the camera 
	//particles typically (not 100% of the time) are billboards
	//the matrices z axis will point towards the x axis is the up axis of the world
	//crossed with the z axis to find a right vector
	//the y axis is the cross of the up and the z axis for the matrix.
	mat3 billboard;
	billboard[2] = normalize( cameraPosition.xyz - P);
	billboard[0] = cross( vec3(0,1,0), billboard[2] );
	billboard[1] = cross( billboard[2], billboard[0]);

	//this is the half size for the billboards, they will be 2x2units in size.
	float halfSize = 2 * 0.5f;
	
	//here we create the vertices for the corners of the billboards
	//P is the centre point, we add on the half size vector multiplied by the 
	//billboard matrix.
	vec3 corners[4];
	corners[0] = P + billboard * vec3( -halfSize, -halfSize, 0 );
	corners[1] = P + billboard * vec3( halfSize, -halfSize, 0 );
	corners[2] = P + billboard * vec3( -halfSize, halfSize, 0 );
	corners[3] = P + billboard * vec3( halfSize, halfSize, 0 );

	
	//set the uv coordiantes for the verts being emitted
	vec2 UVCoords [4];
	UVCoords[0] = vec2( 0.f, 0.f);
	UVCoords[1] = vec2( 1.f, 0.f);
	UVCoords[2] = vec2( 0.f, 1.f);
	UVCoords[3] = vec2( 1.f, 1.f);

	//a simple loop to set the gl_position, set the fragment outputs 
	//and call emitVertex for each corner of the billboard
	for ( int i = 0 ; i < 4 ; ++i )
	{
		vColour = vertex[0].Colour;
		vUV = UVCoords[i];
		gl_Position = ProjectionView * vec4( corners[ i ], 1 );
		//emitVertex is the important call to tell the GL that a vert has been created/processed
		//by the geometry shader
		EmitVertex();
	}
	EndPrimitive();
}