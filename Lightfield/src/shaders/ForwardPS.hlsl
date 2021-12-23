struct Input
{
    float4 worldPos : WorldPos;
    float4 normal : Normal;
    float4 color : Color;
    float4 uvCoords : UvCoords;
};
struct Output
{
    float4 color : SV_Target;
};

#define NUM_LIGHTS 1u
static const float3 lightPosArr[NUM_LIGHTS] =
{
    float3(3.0f, 1.0f, -5.0f)
};

SamplerState samplerState : register(s0);
Texture2D diffuseTexture : register(t0);

Output main(Input input)
{
    Output output;

    output.color = lerp(input.color, diffuseTexture.Sample(samplerState, input.uvCoords.xy), input.uvCoords.z);
    
    // calc lighting
    float lightIntensity = 0.0f;
    
    [unroll]
    for (uint i = 0u; i < NUM_LIGHTS; i++)
    {
        float3 lightDir = lightPosArr[0] - input.worldPos.xyz;
        float dist = length(lightDir);
        float atten = 1.0f / pow(dist, 0.3f);
        lightIntensity += max(dot(input.normal.xyz, normalize(lightDir)), 0.0f) * atten;
    }
    output.color *= max(min(lightIntensity, 1.0f), 0.15f); // 0.1f ambient light
    
    return output;
}