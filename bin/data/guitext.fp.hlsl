#version 140 // OpenGL 3.1 
varying vec2 vCoord;			// vertex texture coordinates 
uniform sampler2D diffuseTex;	// alpha mapped texture 
uniform vec4 diffuseColor;		// actual color for this text 
uniform vec4 outlineColor;		// outline (or shadow) color for the text 

void main(void) 
{ 
	vec2 tex = texture2D(diffuseTex, vCoord).rg;
	
	// color consists of the (diffuse color * main alpha) + (background color * outline alpha)
	vec3 color = (diffuseColor.rgb * tex.r) + (outlineColor.rgb * tex.g);

	// make the main alpha more pronounced, makes small text sharper
	tex.r = clamp(tex.r * 1.5, 0.0, 1.0);

	// alpha is the sum of main alpha and outline alpha
	// main alpha is main font color alpha
	// outline alpha is the stroke or shadow alpha
	float mainAlpha = tex.r * diffuseColor.a;
	float outlineAlpha = tex.g * outlineColor.a * diffuseColor.a;
	gl_FragColor = vec4(color, mainAlpha + outlineAlpha);



	// distance field stuff:
	//vec4 color = texture2D(texture, gl_TexCoord[0].st);
	//float dist = color.a;
	//float width = fwidth(dist);
	//float alpha = smoothstep(0.5-width, 0.5+width, dist);
	//gl_FragColor = vec4(gl_Color.rgb, alpha*gl_Color.a);
}