struct Output
{
    float4 Depth: SV_Target0;
    float4 Stencil: SV_Target1;
};

cbuffer Cam : register(b0) { float4x4 PerspectiveMatrix; };

Texture2D depthBuffer : register(t0);
Texture2D<uint2> stencilBuffer : register(t1);

Output main(float4 screenPos : SV_Position)
{
    const uint2 texPos = uint2(screenPos.xy);

    /// DEPTH STENCIL
    float depth = depthBuffer[texPos].r;
    uint stencil = stencilBuffer[texPos].g;


    // make depth more easily visible in mid-range
    depth = pow(depth, 100);

    Output output;
    output.Depth = float4(depth, depth, depth, 1.0f);
    output.Stencil = float4(stencil, stencil, stencil, 1.0f);
    return output;
}