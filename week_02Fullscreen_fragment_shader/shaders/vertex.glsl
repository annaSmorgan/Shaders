#version 330

in vec4 position;
in vec4 colour;

out vec4 vColour;

void main() 
{
	vColour = colour;
	gl_Position =  position;
}
