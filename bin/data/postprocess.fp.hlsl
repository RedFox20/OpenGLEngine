#version 140 // OpenGL 3.1 
  
uniform sampler2D diffuseTex;	// texture to make glow
varying vec2 vCoord; // texture coordinate for this pixel

vec4 blurEffect(int xSamples, int ySamples)
{
	vec4 sum = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int x = -xSamples; x <= xSamples; ++x)
		for(int y = -ySamples; y <= ySamples; ++y)
			sum += texture2D(diffuseTex, vCoord + vec2(x * 0.005, y * 0.005));
	int div = (xSamples+xSamples+1) * (ySamples+ySamples+1) + 5;
	return sum / div;
}

void main()
{
	vec4 color = texture2D(diffuseTex, vCoord);
	vec4 blurred = blurEffect(1, 1);


	//gl_FragColor = (blurred + color) - (blurred * color); // screen blending
	gl_FragColor = blurred + color;
}