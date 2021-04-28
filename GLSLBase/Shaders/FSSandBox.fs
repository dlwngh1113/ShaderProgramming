#version 450

layout(location=0) out vec4 FragColor;

varying vec4 v_Color;

uniform vec3 u_Point;
uniform vec3 u_Points[10];
uniform float u_Time;

const vec3 Circle = vec3(0.5, 0.5, 0);
const float PI = 3.141592;

vec4 CenteredCircle()
{
    float d = length(v_Color.rgb - Circle);
    float count = 3;
    float rad = d * 2.0 * 2.0 * PI * count;
    float greyScale = sin(rad);
    float width = 200;
    greyScale = pow(greyScale, width);
    return vec4(greyScale);
}

vec4 IndicatePoint()
{
    vec4 returnColor = vec4(0);
    float d = length(v_Color.rg - u_Point.xy);
    if(d < u_Point.z)
    {
        returnColor = vec4(1);
    }
    return returnColor;
}

vec4 IndicatePoints()
{
    vec4 returnColor = vec4(0);
    for(int i=0;i<10;++i)
    {
        float d = length(v_Color.rg - u_Points[i].xy);
        if(d < u_Points[i].z)
        {
            returnColor = vec4(1 - d * 100);
        }
    }
    return returnColor;
}

vec4 Radar()
{
    float d = length(v_Color.rg - vec2(0, 0));
    vec4 returnColor = vec4(0);
    returnColor.a = 0.2;
    float ringRadius = mod(u_Time, 0.7);
    float radarWidth = 0.08;

    if(d > ringRadius && d < ringRadius + radarWidth)
    {
        returnColor = vec4(0.5);
        for(int i=0;i<10;++i)
        {
            float ptDistance = length(u_Points[i].xy - v_Color.rg);
            if(ptDistance < 0.05)
            {
                ptDistance = 0.05 - ptDistance;
                ptDistance *= 20;
                returnColor += vec4(ptDistance);
            }
        }
    }
    return returnColor;
}

vec4 wave()
{
    vec4 returnColor = vec4(0);

    for(int i=0;i<10;++i)
    {
        vec2 ori = u_Points[i].xy;
        vec2 pos = v_Color.rg;
        float dist = length(ori - pos);
        float prequency = 8;

        returnColor += 0.5 * vec4(sin(dist * 2.0 * PI * prequency - u_Time));
    }

    //returnColor = normalize(returnColor);
    return returnColor;
}

void main()
{
    //FragColor = CenteredCircle();
    //FragColor = IndicatePoint();
    //FragColor = IndicatePoints();
    //FragColor = Radar();
    FragColor = wave();
}