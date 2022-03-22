Texture2D gradientBuffer : register(t0);

float main(float4 screenPos : SV_Position) : SV_Target
{
	int2 texPos = int2(screenPos.xy);

	// accumulate values during for loop
	float a = 0.0f;
	float b = 0.0f;

	int s = 1;
	for (int x = -s; x <= s; x++) {
		for (int y = -s; y <= s; y++) {
			int2 offset = int2(x, y);
			float4 gradients = gradientBuffer[texPos + offset]; // Lx, Ly, Lu, Lv

			a += gradients.x * gradients.z + gradients.y * gradients.w;
			b += gradients.x * gradients.x + gradients.y * gradients.y;
		}
	}

	// s = Ep(Lx * Lu + Ly * Lv) / Ep(Lx * Lx + Ly * Ly)
	return (a / b) * 1.0f;
}