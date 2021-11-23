struct Input
{
    float4 normal : Normal;
    float4 color : Color;
};
struct Output
{
    float4 color : SV_Target;
};

Output main(Input input)
{
    Output output;
    output.color = input.color;
    return output;
}