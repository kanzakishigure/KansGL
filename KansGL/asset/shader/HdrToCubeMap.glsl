#type vertex
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = aPos;
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}

#type fragment
#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

//使用local position 得到uv
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    //opengl中y轴向上
    //单位球，半径为1
    //球坐标系 θ = arctanz/x
    //φ = asiny/1.0
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    //不知道缩放原因，雅各比矩阵？
    uv *= invAtan;
    //保证uv从0.0-1.0
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}