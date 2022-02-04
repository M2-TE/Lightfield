Texture2D colorBufferA : register(t0); // left eye
Texture2D colorBufferB : register(t1); // right eye

#define a 0.229879f
#define b 0.540242f
#define c 0.425287f
static const float3 p = float3(a, b, a);
static const float3 d = float3(-c, 0.0f, c); // is the minus really correct in the paper?

// convert rgb to luma, as perceived by our eyes
// need to convert from sRGB (gamma) to rgb (linear) or use sRGB texture buffers (ofc only when the incoming image is in sRGB color format)
#define LUMA_DIGITAL_ITU_BT709(rgb) 0.2126f * rgb.r + 0.7152f * rgb.g + 0.0722f * rgb.b
#define LUMA_DIGITAL_ITU_BT601(rgb) 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b
#define LUMA_DIGITAL_ITU_BT601_PRECISE(rgb) sqrt(0.299 * rgb.r * rgb.r + 0.587 * rgb.g * rgb.g + 0.114 * rgb.b * rgb.b)
// simple greyscale by averaging channels
#define LUMINANCE(rgb) (rgb.r + rgb.g + rgb.b) * 0.333333f

float main(float4 screenPos : SV_Position) : SV_Target
{
    const uint2 texPos = uint2(screenPos.xy);
    float4 colorA = colorBufferA[texPos];
    float4 colorB = colorBufferB[texPos];

    return (colorA + colorB) / 2.0f;
}