#version 450

layout(location=0) out vec4 FragColor;

uniform sampler2D u_TexSampler;

in vec2 v_TexPos;

const float PI = 3.141592;

void main()
{
	vec2 newPos = v_TexPos;
	newPos.y = max(newPos.y, -newPos.y + 1.0) * 2;
	FragColor = texture(u_TexSampler, newPos);
}