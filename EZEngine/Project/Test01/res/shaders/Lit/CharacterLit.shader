#shader vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in ivec4 a_BoneIndices;
layout(location = 4) in vec4 a_BoneWeights;

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

layout(std430, binding = 1) readonly buffer BoneBuffer
{
    mat4 boneMatrices[];
};

uniform mat4 u_ViewProj;

out vec3 v_WorldPos;
out vec3 v_WorldNormal;
out vec2 v_TexCoord;

void main()
{
    InstanceData inst = instances[gl_InstanceID];
    mat4 model = inst.model;
    uint paletteBaseOffset = inst.paletteBaseOffset;

    vec4 skinnedPosition = vec4(0.0);
    vec3 skinnedNormal = vec3(0.0);
    float sumWeight = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        int idx = a_BoneIndices[i];
        float w = a_BoneWeights[i];

        if (w > 0.0 && idx >= 0)
        {
            mat4 boneMat = boneMatrices[paletteBaseOffset + uint(idx)];
            skinnedPosition += w * (boneMat * vec4(a_Position, 1.0));
            skinnedNormal += w * (mat3(boneMat) * a_Normal);
            sumWeight += w;
        }
    }

    if (sumWeight <= 0.0)
    {
        skinnedPosition = vec4(a_Position, 1.0);
        skinnedNormal = a_Normal;
    }

    vec4 worldPos = model * skinnedPosition;

    v_WorldPos = worldPos.xyz;
    v_WorldNormal = normalize(mat3(model) * skinnedNormal);
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

// 二次元补一点 rim
uniform vec3 u_RimColor;
uniform float u_RimPower;
uniform vec3 u_CameraPos;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(v_WorldNormal);
    vec3 L = normalize(-u_LightDir);
    vec3 V = normalize(u_CameraPos - v_WorldPos);

    vec4 base = u_BaseColor;
    if (u_HasBaseColorTex != 0) {
        base *= texture(u_BaseColorTex, v_TexCoord);
    }

    float ndotl = max(dot(N, L), 0.0);

    // 二段式更适合二次元
    float toon = ndotl > 0.5 ? 1.0 : 0.55;

    float rim = pow(1.0 - max(dot(N, V), 0.0), max(u_RimPower, 0.001));

    vec3 lit = base.rgb * (u_AmbientColor + toon * u_LightColor);
    lit += rim * u_RimColor;

    FragColor = vec4(lit, base.a);
}