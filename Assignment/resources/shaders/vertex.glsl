#version 150

in vec4 Position;
in vec4 Colour;
in vec4 Normal;
in vec2 Tex1;

out Vertex//data about the vertex 
{
    vec4 Pos;
    vec4 Normal;
    vec4 Colour; 
    vec2 UV;
    vec4 ShadowCoord;
    vec3 eyeDir;

}vertex;
//uniform matricies
uniform mat4 ProjectionView; 
uniform mat4 Model;
uniform mat4 NormalMatrix;
uniform mat4 ViewMatrix;
uniform mat4 shadowProjectionView;

uniform vec4 cameraPosition;

const mat4 SHADOW_BIAS_MATRIX = mat4(
                            0.5, 0.0, 0.0, 0.0,
                            0.0, 0.5, 0.0, 0.0,
                            0.0, 0.0, 0.5, 0.0,
                            0.5, 0.5, 0.5, 1.0
                            );

void main() 
{ 
    //vertex variables
    vertex.Pos = Model * Position;
    vertex.Normal = Model * NormalMatrix * Normal;
    vertex.Colour = Colour;
    vertex.UV = Tex1;

    vertex.eyeDir = (ViewMatrix * normalize( cameraPosition - vertex.Pos)).xyz;

    vertex.ShadowCoord = SHADOW_BIAS_MATRIX * shadowProjectionView * Position;

    gl_Position = ProjectionView * Model * Position;
}