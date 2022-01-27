Texture2D colorBuffer : register(t0);

float main(float4 screenPos : SV_Position) : SV_Target
{
    uint2 texPos = uint2(screenPos.xy);

    // TODO

    return 0.5f;
}