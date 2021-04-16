#version 450

in vec3 a_Position;
in vec3 a_Velocity;
in float a_EmitTime;
in float a_LifeTime;
in float a_P;
in float a_A;

uniform float u_Time; //누적 시간

const vec3 c_Gravity = vec3(0, 0, 0);

void main()
{
	float newTime = u_Time - a_EmitTime;
	
	vec3 newPos = a_Position;

	if(newTime < 0.0)
	{
		newPos = vec3(1000, 10000, 10000);
	}
	else
	{
		newTime = mod(newTime, a_LifeTime);
		newPos = newPos + vec3(newTime, 0, 0);
		newPos.y = newPos.y + a_A * newTime * sin(newTime * 3.14 * a_P);
		//float t = newTime;
		//float tt = newTime * newTime;
		//newPos = newPos + u_Time * a_Velocity + 0.5 * c_Gravity * tt;
	}

	gl_Position = vec4(newPos, 1);
}