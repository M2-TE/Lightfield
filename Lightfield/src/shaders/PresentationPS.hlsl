Texture2D<float4> colorBuffer : register(t0);
Texture2D<float> simulatedDepthBuffer : register(t1);
Texture2D<float> outputDepthBuffer : register(t2);

cbuffer PresentationModeBuffer : register(b0) { uint iPresentationMode; };

float4 main(float4 screenPos : SV_Position) : SV_Target
{
    const uint2 texPos = uint2(screenPos.xy);

    if (iPresentationMode == 0) { // COLOR
        float4 color = colorBuffer[texPos];
        return color;
    }
    else if (iPresentationMode == 1) { // SIMULATED DEPTH
        float depth = simulatedDepthBuffer[texPos];
        depth /= 20.0f; // should probably be scaled dynamically instead
        return float4(depth, depth, depth, 1.0f);
    }
    else if (iPresentationMode == 2) { // OUTPUT DEPTH
        float depth = outputDepthBuffer[texPos];
        return float4(depth, depth, depth, 1.0f);
    }
    else {
        return float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
}