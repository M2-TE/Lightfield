cbuffer ObjectModelBuffer       : register(b0) { float4x4 ModelMatrix; };
cbuffer CameraViewBuffer        : register(b1) { float4x4 ViewMatrix; };
cbuffer CameraProjectionBuffer  : register(b2) { float4x4 ProjectionMatrix; };

struct Input
{
    float4 pos : Position;
    float4 normal : Normal;
    float4 color : Color;
    float4 uvCoords : UvCoords;
};
struct Output
{
    float4 worldPos : WorldPos;
    float4 normal : Normal;
    float4 color : Color;
    float4 uvCoords : UvCoords;
    
    float4 screenPos : SV_Position;
};

Output main(Input input)
{
    Output output;
    output.worldPos = mul(input.pos, ModelMatrix);
    output.screenPos = mul(output.worldPos, ViewMatrix);
    output.screenPos = mul(output.screenPos, ProjectionMatrix);
    
    output.normal = mul(input.normal, ModelMatrix);
    output.normal = normalize(output.normal); // potentially not necessary
    
    output.color = input.color;

    output.uvCoords = input.uvCoords;

	return output;
}