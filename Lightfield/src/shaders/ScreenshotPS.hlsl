cbuffer Cam : register(b0) { float4x4 PerspectiveMatrix; };

Texture2D depthBuffer : register(t0);

float4 main(float4 screenPos : SV_Position) : SV_Target
{
    const uint2 texPos = uint2(screenPos.xy);

    float depth = depthBuffer[texPos].r;

    // make depth more easily visible in mid-range
    depth = pow(depth, 100);

    return float4(depth, depth, depth, 1.0f);;
}