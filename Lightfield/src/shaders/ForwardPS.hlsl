struct Input
{
    float4 worldPos : WorldPos;
    float4 viewPos : ViewPos;
    float4 normal : Normal;
    float4 color : Color;
    float4 uvCoords : UvCoords;
};
struct Output
{
    float4 color : SV_Target0;
    float depth : SV_Target1;
};

#define NUM_LIGHTS 1u
static const float3 lightPosArr[NUM_LIGHTS] = { float3(0.0f, 0.0f, 0.0f) };

SamplerState samplerState : register(s0);
Texture2D diffuseTexture : register(t0);

Output main(Input input)
{
    Output output;

    // switch between texture and vertex colors based on z coord
    output.color = lerp(input.color, diffuseTexture.Sample(samplerState, input.uvCoords.xy), input.uvCoords.z);
    
    // calc lighting
    float lightIntensity = 0.0f;
    
    [unroll]
    for (uint i = 0u; i < NUM_LIGHTS; i++)
    {
        //float3 lightDir = lightPosArr[0] - input.worldPos.xyz;
        float3 lightDir = float3(0.0f, 0.0f, -1.0f); // DEBUG: SIMULATED SUN FOR CONSTANT LIGHT DIR
        float dist = length(lightDir);
        float atten = 1.0f / pow(dist, 0.3f);
        lightIntensity += max(dot(input.normal.xyz, normalize(lightDir)), 0.1f) * atten;
    }
    output.color *= max(min(lightIntensity, 1.0f), 0.15f); // 0.1f ambient light

    // calculate distance from camera to object via magnitude of view pos
    output.depth = length(input.viewPos.xyz);
    output.depth /= 20.0f; // should probably be scaled dynamically instead

    return output;
}