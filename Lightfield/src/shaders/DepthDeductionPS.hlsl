Texture2D colorBufferA : register(t0); // left eye
Texture2D colorBufferB : register(t1); // right eye

// convert rgb to luma, as perceived by our eyes
// need to convert from sRGB (gamma) to rgb (linear) or use sRGB texture buffers (ofc only when the incoming image is in sRGB color format)
#define LUMA_DIGITAL_ITU_BT709(col) 0.2126f * col.x + 0.7152f * col.y + 0.0722f * col.z
#define LUMA_DIGITAL_ITU_BT601(col) 0.299 * col.x + 0.587 * col.y + 0.114 * col.z
#define LUMA_DIGITAL_ITU_BT601_PRECISE(col) sqrt(0.299 * col.x * col.x + 0.587 * col.y * col.y + 0.114 * col.z * col.z)
// simple greyscale by averaging channels
#define LUMINANCE(col) dot(col, float3(0.333333f, 0.333333f, 0.333333f));

// final define to be used in code
#define BRIGHTNESS(col) LUMINANCE(col) // using standard greyscale for now, human perception might be fun to research later

float main(float4 screenPos : SV_Position) : SV_Target
{
    const int2 texPos = int2(screenPos.xy);

    // lightfield derivatives
    float Lx = 0.0f;
    float Ly = 0.0f;
    float Lu = 0.0f;
    float Lv = 0.0f;

    float P = 0.0f;

    // calc derivatives using color inputs
    int s = 1;
    for (int x = -s; x <= s; x++) {
        for (int y = -s; y <= s; y++) {

            const int2 texOffset = int2(x, y);

            // read texture
            const float3 colorA = colorBufferA[texPos - texOffset].rgb;
            const float3 colorB = colorBufferB[texPos - texOffset].rgb;
            const float lumaA = BRIGHTNESS(colorA);
            const float lumaB = BRIGHTNESS(colorB);

            // pulled them out of static context for now
            uint u = 0u;
            uint v = 1u;

            float3 p = float3(0.229879f, 0.540242f, 0.229879f);
            float3 d = float3(-0.425287f, 0.0f, 0.425287f);

            // calculate (approximate) derivatives
            uint2 i = uint2(x + 1, y + 1);
            Lx += d[i.x] * p[i.y] * p[u] * p[v] * lumaA;
            Ly += p[i.x] * d[i.y] * p[u] * p[v] * lumaA;
            Lu += p[i.x] * p[i.y] * d[u] * p[v] * lumaA;
            Lv += p[i.x] * p[i.y] * p[u] * d[v] * lumaA;

            u = 2u;
            d *= -1.0f; // flip direction of d vector to point to middle
            Lx += d[i.x] * p[i.y] * p[u] * p[v] * lumaB;
            Ly += p[i.x] * d[i.y] * p[u] * p[v] * lumaB;
            Lu += p[i.x] * p[i.y] * d[u] * p[v] * lumaB;
            Lv += p[i.x] * p[i.y] * p[u] * d[v] * lumaB;

            // following the paper here, not sure how this works
            P += lumaA + lumaB;
        }
    }

    // is P just the accumulated central luma?
    //P = 0.0f;
    for (int x = -s; x <= s; x++) {
        for (int y = -s; y <= s; y++) {
            // TODO
        }
    }

    float disparity = (P * (Lx * Lu + Ly * Lv)) / (P * (Lx * Lx + Ly * Ly));
    return disparity;


    //return confidence;
    //return disparity;
}