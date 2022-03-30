#version 150

in vec4 Position; 
in vec4 Colour;

//We can output a structure from a shader as well
//we do not need to only output basic native types
out Vertex
{
	vec4 Colour;
} vertex;

//This is a simple pass through shader that simply emitts the vertex position
//There is not model or view space transform carried out here either
void main() 
{ 
	vertex.Colour = Colour;
	gl_Position = Position; 
}