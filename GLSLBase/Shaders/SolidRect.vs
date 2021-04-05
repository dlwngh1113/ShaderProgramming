#version 450

in vec3 Position;
in vec3 Position1;

uniform float u_Scale;

void main()
{
	gl_Position = vec4(Position * u_Scale, 1);
}