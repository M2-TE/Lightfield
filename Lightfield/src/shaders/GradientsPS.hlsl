#define BRIGHTNESS(col) dot(col, float3(0.333333f, 0.333333f, 0.333333f)); // using standard greyscale for now, human perception might be fun to look into later

Texture2D colorBufferA : register(t0); // left eye
Texture2D colorBufferB : register(t1); // right eye

Texture2DArray colBuffArr : register(t0);

float4 main(float4 screenPos : SV_Position) : SV_Target
{
    const int3 texPos = uint3(screenPos.xy, 0);

    // lightfield derivatives
    float Lx = 0.0f;
    float Ly = 0.0f;
    float Lu = 0.0f;
    float Lv = 0.0f;

    // calc derivatives using color inputs
    const int s = 1; // 3x3, -1 -> 1, x and y
    const uint k = 2; // 3x3, 0 -> 2, u and v

    // iterate over 2D patch of pixels
    for (int x = -s; x <= s; x++) {
        for (int y = -s; y <= s; y++) {

            // iterate over "2D" patch of cameras
            int camIndex = 0; // keep track of 1D index into camera array
            for (uint u = 0u; u <= k; u++) {
                for (uint v = 0u; v <= k; v++) {
                    const int3 texOffset = int3(x, y, camIndex++);

                    const float3 color = colBuffArr[uint3(texPos - texOffset)].rgb;
                    const float luma = BRIGHTNESS(color);

                    float3 p = float3(0.229879f, 0.540242f, 0.229879f);
                    // flipping to always point to center camera
                    float3 d = u < 1u ? float3(-0.425287f, 0.0f, 0.425287f) : float3(0.425287f, 0.0f, -0.425287f);

                    // calculate (approximate) derivatives
                    uint2 i = uint2(x + s, y + s);
                    Lx += d[i.x] * p[i.y] * p[u] * p[v] * luma;
                    Ly += p[i.x] * d[i.y] * p[u] * p[v] * luma;
                    Lu += p[i.x] * p[i.y] * d[u] * p[v] * luma;
                    Lv += p[i.x] * p[i.y] * p[u] * d[v] * luma;
                }
            }
        }
    }

    return float4(Lx, Ly, Lu, Lv);
}