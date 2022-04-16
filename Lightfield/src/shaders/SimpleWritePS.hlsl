Texture2DArray colBuffArr : register(t0);
cbuffer CamIndexBuffer : register(b0) { uint iCam; };

float4 main(float4 screenPos : SV_Position) : SV_Target
{
	const uint3 texPos = uint3(screenPos.x, screenPos.y, iCam);
	return colBuffArr[texPos];
}