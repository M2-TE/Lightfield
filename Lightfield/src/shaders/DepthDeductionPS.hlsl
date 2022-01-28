Texture2D colorBuffer : register(t0);

float main(float4 screenPos : SV_Position) : SV_Target
{
    const uint2 texPos = uint2(screenPos.xy);
    float4 color = colorBuffer[texPos];

    // TODO: skip alpha=0.0f pixels

    return color.r;
}