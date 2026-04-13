#shader vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

struct InstanceData
{
    mat4 model;
    uint paletteBaseOffset;
    uint _pad0;
    uint _pad1;
    uint _pad2;
};

layout(std430, binding = 0) readonly buffer InstanceBuffer
{
    InstanceData instances[];
};

uniform mat4 u_ViewProj;

out vec3 v_WorldPos;
out vec3 v_WorldNormal;
out vec2 v_TexCoord;

void main()
{
    mat4 model = instances[gl_InstanceID].model;
    vec4 worldPos = model * vec4(a_Position, 1.0);

    v_WorldPos = worldPos.xyz;
    v_WorldNormal = normalize(mat3(model) * a_Normal);
    v_TexCoord = a_TexCoord;

    gl_Position = u_ViewProj * worldPos;
}

#shader fragment
#version 430 core

in vec3 v_WorldPos;
in vec3 v_WorldNormal;
in vec2 v_TexCoord;

uniform vec3 u_LightDir;
uniform vec3 u_LightColor;
uniform vec3 u_AmbientColor;
uniform vec4 u_BaseColor;
uniform sampler2D u_BaseColorTex;
uniform int u_HasBaseColorTex;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(v_WorldNormal);
    vec3 L = normalize(-u_LightDir);

    vec4 base = u_BaseColor;
    if (u_HasBaseColorTex != 0) {
        base *= texture(u_BaseColorTex, v_TexCoord);
    }

    float ndotl = max(dot(N, L), 0.0);
    vec3 lit = base.rgb * (u_AmbientColor + ndotl * u_LightColor);

    FragColor = vec4(lit, base.a);
}