struct Input
{
    float4 worldPos : WorldPos;
    float4 normal : Normal;
    float4 color : Color;
};
struct Output
{
    float4 color : SV_Target;
};

#define NUM_LIGHTS 1u
static const float3 lightPosArr[NUM_LIGHTS] =
{
    float3(5.0f, 2.0f, -10.0f)
};

Output main(Input input)
{
    Output output;
    output.color = input.color;
    
    // calc lighting
    float lightIntensity = 0.0f;
    
    [unroll]
    for (uint i = 0u; i < NUM_LIGHTS; i++)
    {
        float3 lightDir = normalize(lightPosArr[0] - input.worldPos.xyz);
        lightIntensity += max(dot(input.normal.xyz, lightDir), 0.0f);
    }
    output.color *= min(lightIntensity, 1.0f);
    
    return output;
}