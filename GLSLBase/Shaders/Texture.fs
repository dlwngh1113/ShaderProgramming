#version 450

layout(location=0) out vec4 FragColor;

uniform sampler2D u_TexSampler;

in vec2 v_TexPos;

const float PI = 3.141592;

void main()
{
	vec2 newPos = v_TexPos;
	if(newPos.y > 0.5)
	{
		newPos.y = (newPos.y + 0.5) * 2;
	}
	else
	{
		newPos.y = (-newPos.y - 0.5) * 2;
	}
	FragColor = texture(u_TexSampler, newPos);
}