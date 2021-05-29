#version 450

layout(location=0) out vec4 FragColor;

uniform sampler2D u_TexSampler;

in vec2 v_TexPos;

const float PI = 3.141592;

void main()
{
	vec2 newPos = v_TexPos;
	newPos.y = (floor(newPos.x * 3.0) + newPos.y) / 3.0;
	newPos.x = fract(newPos.x * 3.0);
	FragColor = texture(u_TexSampler, newPos);
}