cbuffer CameraProjectionBuffer  : register(b0) { float4x4 ProjectionMatrix; };
cbuffer CameraViewBuffer        : register(b1) { float4x4 ViewMatrix; };
cbuffer ObjectModelBuffer       : register(b2) { float4x4 ModelMatrix; };

struct Input
{
    float4 pos : Position;
    float4 normal : Normal;
    float4 color : Color;
};
struct Output
{
    float4 normal : Normal;
    float4 color : Color;
    
    float4 screenPos : SV_Position;
};

Output main(Input input)
{
    Output output;
    output.screenPos = mul(input.pos, ModelMatrix);
    output.screenPos = mul(output.screenPos, ViewMatrix);
    output.screenPos = mul(output.screenPos, ProjectionMatrix);
    
    output.normal = mul(input.normal, ModelMatrix);
    output.normal = mul(output.normal, ViewMatrix);
    output.normal = mul(output.normal, ProjectionMatrix);
    
    output.color = input.color;
	return output;
}