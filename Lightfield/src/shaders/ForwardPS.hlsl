struct Input
{
	
};
struct Output
{
    float4 color : SV_Target;
};

Output main(Input input)
{
    Output output;
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}