#version 450

layout(location=0) out vec4 FragColor;

uniform sampler2D u_TexSampler;
uniform float u_Step;

in vec2 v_TexPos;

const float PI = 3.141592;

vec4 SingleTexture()
{
	vec4 temp = texture(u_TexSampler, v_TexPos);
return temp;
}

vec4 SpriteTexture()
{
	vec2 newTex = vec2(v_TexPos.x, u_Step / 6.0 + v_TexPos.y / 6.0);
	return texture(u_TexSampler, newTex);
}

void main()
{
	//vec2 newPos = v_TexPos;
	//newPos.y = (floor(newPos.x * 3.0) + newPos.y) / 3.0;
	//newPos.x = fract(newPos.x * 3.0) - 2.0 / 3.0;
	//FragColor = texture(u_TexSampler, newPos);
	FragColor = SpriteTexture();
}