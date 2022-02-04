Texture2D colorBufferA : register(t0); // left eye
Texture2D colorBufferB : register(t1); // right eye

float main(float4 screenPos : SV_Position) : SV_Target
{
    const uint2 texPos = uint2(screenPos.xy);
    float4 colorA = colorBufferA[texPos];
    float4 colorB = colorBufferB[texPos];

    // TODO: skip alpha=0.0f pixels?

    return colorA.r;
}