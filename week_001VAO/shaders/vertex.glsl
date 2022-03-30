#version 150

in vec4 position;
in vec4 colour;

out vec4 vColour;

uniform mat4 ProjectionView;

void main() 
{
	vColour = colour;
	gl_Position = ProjectionView * position;
}
