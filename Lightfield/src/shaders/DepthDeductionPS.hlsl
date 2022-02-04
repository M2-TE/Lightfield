Texture2D colorBufferA : register(t0); // left eye
Texture2D colorBufferB : register(t1); // right eye

#define a 0.229879f
#define b 0.540242f
#define c 0.425287f
static const float3 p = float3(a, b, a);
static const float3 d = float3(-c, 0.0f, c); // is the -c really correct in the paper?

// convert rgb to luma, as perceived by our eyes
// need to convert from sRGB (gamma) to rgb (linear) or use sRGB texture buffers (ofc only when the incoming image is in sRGB color format)
#define LUMA_DIGITAL_ITU_BT709(col) 0.2126f * col.x + 0.7152f * col.y + 0.0722f * col.z
#define LUMA_DIGITAL_ITU_BT601(col) 0.299 * col.x + 0.587 * col.y + 0.114 * col.z
#define LUMA_DIGITAL_ITU_BT601_PRECISE(col) sqrt(0.299 * col.x * col.x + 0.587 * col.y * col.y + 0.114 * col.z * col.z)
// simple greyscale by averaging channels
#define LUMINANCE(col) col.rgb * 0.333333f

// final define to be used in code
#define BRIGHTNESS(col) LUMINANCE(col) // using greyscale for now, human perception might be fun to research later

float main(float4 screenPos : SV_Position) : SV_Target
{
    const int2 texPos = int2(screenPos.xy);

    // lightfield derivatives
    float Lx = 0.0f;
    float Ly = 0.0f;
    float Lu = 0.0f;
    float Lv = 0.0f;

    // calc derivatives using color inputs
    for (int l = -1; l < 2; l++) {
        for (int j = -1; j < 2; j++) {

            const int il = int(l + 1);
            const int ij = int(j + 1);
            const int2 lj = int2(l, j);
            
            const float4 colorA = colorBufferA[texPos + lj];
            const float4 colorB = colorBufferB[texPos + lj];
            const float lumaA = BRIGHTNESS(colorA);
            const float lumaB = BRIGHTNESS(colorB);

            Lx += d[il] * p[ij] * p[0] * p[1] * lumaA;
            Lx += d[il] * p[ij] * p[2] * p[1] * lumaB;

            Ly += p[il] * d[ij] * p[0] * p[1] * lumaA;
            Ly += p[il] * d[ij] * p[2] * p[1] * lumaB;

            Lu += p[il] * p[ij] * d[0] * p[1] * lumaA;
            Lu += p[il] * p[ij] * d[2] * p[1] * lumaB;

            Lv += p[il] * p[ij] * p[0] * d[1] * lumaA;
            Lv += p[il] * p[ij] * p[2] * d[1] * lumaB;
        }   
    }

    return Lx + Ly + Lu + Lv; // forcing compiler to actually compile the code

    // TEMP
    //float4 colorA = colorBufferA[texPos];
    //float4 colorB = colorBufferB[texPos];
    //return (colorA + colorB) / 2.0f;
}