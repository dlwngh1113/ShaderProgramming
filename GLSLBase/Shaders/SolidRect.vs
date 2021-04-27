#version 450

in vec3 a_Position;
in vec3 a_Velocity;
in float a_EmitTime;
in float a_LifeTime;
in float a_P;
in float a_A;
in float a_randValue;
in vec4 a_Color;

uniform float u_Time; //누적 시간
uniform vec3 u_ExForce;

const vec3 c_Gravity = vec3(0, -0.8, 0);
const mat3 c_NV = mat3(0, -1, 0, 1, 0, 0, 0, 0, 0);

out vec4 v_Color;

void main()
{
	float newTime = u_Time - a_EmitTime;
	
	//vec3 newPos = a_Position;
	vec3 newPos;
	newPos.x = a_Position.x + cos(a_randValue * 2 * 3.14) * 0.1;
	newPos.y = a_Position.y + sin(a_randValue * 2 * 3.14) * 0.1;

	vec4 color = vec4(0);

	if(newTime < 0.0)
	{
		newPos = vec3(10000, 10000, 10000);
	}
	else
	{
		/*newTime = mod(newTime, a_LifeTime);
		newPos = newPos + vec3(newTime, 0, 0);
		newPos.y = newPos.y + a_A * newTime * sin(newTime * 3.14 * a_P);*/

		newTime = mod(newTime, a_LifeTime);
		float t = newTime;
		float tt = newTime * newTime;
		vec3 newAcc = c_Gravity + u_ExForce;
		vec3 currVel = a_Velocity + t * newAcc;
		vec3 normalV = normalize(currVel * c_NV);
		newPos = newPos + t * a_Velocity + 0.5 * newAcc * tt;
		newPos = newPos + normalV * a_A * sin(newTime * 2 * 3.14 * a_P);

		float intensity  = 1.0 - t / a_LifeTime;
		color = a_Color * intensity;
		intensity = sin(2 * 3.14 * t) * 0.5f + 0.5f;
		color.a = intensity;
	}

	gl_Position = vec4(newPos, 1);
	v_Color = color;
}