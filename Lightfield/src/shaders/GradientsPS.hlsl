#define BRIGHTNESS(col) dot(col, float3(0.333333f, 0.333333f, 0.333333f)); // using standard greyscale for now, human perception might be fun to look into later

Texture2DArray colBuffArr : register(t0);

float4 main(float4 screenPos : SV_Position) : SV_Target
{
    const int3 texPos = int3(screenPos.xy, 0);

    // lightfield derivatives
    float Lx = 0.0f;
    float Ly = 0.0f;
    float Lu = 0.0f;
    float Lv = 0.0f;

    // calc derivatives using color inputs
    const int k = 2; // 3x3, 0 -> 2
    const int kH = 1; // k / 2

    const float3 p = float3(0.229879f, 0.540242f, 0.229879f);
    const float3 dA = float3(-0.425287f, 0.0f, 0.425287f);
    const float3 dB = float3(0.425287f, 0.0f, -0.425287f);

    // iterate over 2D patch of pixels
    for (int x = 0; x <= k; x++) {
        for (int y = 0; y <= k; y++) {
            for (int u = 0; u <= k; u++) {
                for (int v = 0; v <= k; v++) {

                    int camIndex = u * k + v;
                    int3 texOffset = int3(x - kH, y - kH, 0);

                    float3 color = colBuffArr[uint3(texPos + texOffset + int3(0, 0, camIndex))].rgb;
                    float luma = BRIGHTNESS(color);

                    // approximate derivatives using 3-tap filter
                    float3 d;
                    d = u < 1 ? dA : dB;
                    Lx += d[x] * p[y] * p[u] * p[v] * luma;
                    d = x < 1 ? dA : dB;
                    Lu += p[x] * p[y] * d[u] * p[v] * luma * 0.1f;

                    d = v < 1 ? dA : dB;
                    Ly += p[x] * d[y] * p[u] * p[v] * luma;
                    d = y < 1 ? dA : dB;
                    Lv += p[x] * p[y] * p[u] * d[v] * luma * 0.1f;
                }
            }
        }
    }

    return float4(Lx, Ly, Lu, Lv);
}