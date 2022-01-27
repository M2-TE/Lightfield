Texture2D<float4> colorBuffer : register(t0);
Texture2D<float> depthBuffer : register(t0);

// TODO: presentation mode index

float4 main(float4 screenPos : SV_Position) : SV_Target
{
    const uint2 texPos = uint2(screenPos.xy);

    float4 color = colorBuffer[texPos];
    float depth = depthBuffer[texPos];

    //return float4(depth, 1.0f, 1.0f, 1.0f);
    return color;
}