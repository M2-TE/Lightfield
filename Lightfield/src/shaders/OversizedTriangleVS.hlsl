// rendering oversized triangle
float4 main(uint vertID : SV_VertexID) : SV_POSITION
{
    return float4((vertID == 1) ? 3.0f : -1.0f, (vertID == 2) ? -3.0f : 1.0f, 0.0f, 1.0f);
}