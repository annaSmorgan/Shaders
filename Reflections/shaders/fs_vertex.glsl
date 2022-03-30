#version 150

in vec4 Position;
in vec2 Tex1;

out vec2 UV;

uniform mat4 ProjectionView;
uniform mat4 Model;

void main() 
{ 
	UV = Tex1;
	gl_Position = ProjectionView * Model * Position;
}