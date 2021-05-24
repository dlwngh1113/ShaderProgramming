#version 450

in vec3 a_Position;

uniform float u_Time; //���� �ð�

const float PI = 3.141592;

void main()
{
	vec3 newPos = vec3(0);

	float sinValue = 2 * PI * (a_Position.x + 0.5);
	newPos.x = a_Position.x;
	newPos.y = a_Position.y + 0.2 * sin(sinValue + u_Time + a_Position.x);
	gl_Position = vec4(newPos, 1);
}