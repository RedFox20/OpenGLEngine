#define WHATEVER 1
#version 140 // OpenGL 3.1 
//varying vec2 vCoord;			// vertex texture coordinates 
//uniform sampler2D diffuseTex;	// alpha mapped texture 
//uniform vec4 diffuseColor;		// actual color for this text 
//uniform vec4 outlineColor;		// outline (or shadow) color for the text 
//
//void main(void) 
//{ 
//	vec2 tex = texture2D(diffuseTex, vCoord).rg;
//	
//	// color consists of the (diffuse color * main alpha) + (background color * outline alpha)
//	vec3 color = (diffuseColor.rgb * tex.r) + (outlineColor.rgb * tex.g);
//
//	// make the main alpha more pronounced, makes small text sharper
//	tex.r = clamp(tex.r * 1.5, 0.0, 1.0);
//
//	// alpha is the sum of main alpha and outline alpha
//	// main alpha is main font color alpha
//	// outline alpha is the stroke or shadow alpha
//	float mainAlpha = tex.r * diffuseColor.a;
//	float outlineAlpha = tex.g * outlineColor.a * diffuseColor.a;
//	gl_FragColor = vec4(color, mainAlpha + outlineAlpha);
//
//
//
//	// distance field stuff:
//	//vec4 color = texture2D(texture, gl_TexCoord[0].st);
//	//float dist = color.a;
//	//float width = fwidth(dist);
//	//float alpha = smoothstep(0.5-width, 0.5+width, dist);
//	//gl_FragColor = vec4(gl_Color.rgb, alpha*gl_Color.a);
//}

//varying vec2 vCoord;			// vertex texture coordinates 
//uniform sampler2D diffuseTex;	// alpha mapped texture 
//uniform vec4 diffuseColor;		// actual color for this text 
//uniform vec4 outlineColor;		// outline (or shadow) color for the text 
//const float glyph_center   = 0.50;
//const float outline_center = 0.65;
//const float glow_center    = 1.25;
//const vec3 glowColor = vec3(0.1f, 0.1f, 0.1f);
//void main(void)
//{
//	float dist  = texture2D( diffuseTex, vCoord).r;
//	float width = fwidth(dist);
//	float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);
//
//    // Smooth
//	gl_FragColor = vec4(diffuseColor.rgb, alpha);
//
//	//// Outline
//	//float mu = smoothstep(outline_center-width, outline_center+width, dist);
//	//vec3 rgb = mix(outlineColor.rgb, diffuseColor.rgb, mu);
//	//gl_FragColor = vec4(rgb, max(alpha,mu));
//
//    // Glow
//    //vec3 rgb = mix(glow_color, glyph_color, alpha);
//    //float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
//    //gl_FragColor = vec4(rgb, max(alpha,mu));
//
//    //// Glow + outline
//    //vec3 rgb = mix(glowColor.rgb, diffuseColor.rgb, alpha);
//    //float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
//    //vec4 color = vec4(rgb, max(alpha,mu));
//    //float beta = smoothstep(outline_center-width, outline_center+width, dist);
//    //rgb = mix(outlineColor.rgb, color.rgb, beta);
//    //gl_FragColor = vec4(rgb, max(color.a,beta));
//}

//// @note BEST looking function so far:
//varying vec2 vCoord;			// vertex texture coordinates 
//uniform sampler2D diffuseTex;	// alpha mapped texture 
//uniform vec4 diffuseColor;		// actual color for this text 
//uniform vec4 outlineColor;		// outline (or shadow) color for the text 
//const float delta = 0.08f;
//void main( void )
//{
//	// get the alpha value from the distance field texture
//	float rawAlpha = texture2D( diffuseTex, vCoord).r;
//
//	if ((rawAlpha - (0.5f - delta)) < 0.0f) discard;
//
//	gl_FragColor = vec4(diffuseColor.rgb, smoothstep(0.5f-delta,0.5f+delta,rawAlpha));
//}

varying vec2 vCoord;			// vertex texture coordinates
varying vec2 vPixel;            // size of a single pixel in fraction 1.0/textureSize
uniform sampler2D diffuseTex;	// alpha mapped texture 
uniform vec4 diffuseColor;		// actual color for this text 
uniform vec4 outlineColor;		// outline (or shadow) color for the text
const float diffuse_end    = 0.51f;  // distance where diffuse color ends (smaller is farther)
const float diffuse_smooth = 0.05f;  // smoothing for diffuse edges
const float outline_radii  = 0.15f;
const float outline_smooth = 0.05f;
const float outline_end    = diffuse_end - outline_radii;


void main(void)
{
	//// get the alpha value from the distance field texture
	//float distance = texture2D( diffuseTex, vCoord).r;

	//if ((distance - 0.45f) < 0.0f) discard;

	//gl_FragColor = vec4(diffuseColor.rgb, smoothstep(0.45f,0.55f,distance));

	// get the distance value from the distance field texture
	// 1.0 - actually 0 distance from glyph edge
	// 0.0 - very far from glyph edge
	float distance = texture2D(diffuseTex, vCoord).r; 

	float diff_alpha = smoothstep(diffuse_end - diffuse_smooth, diffuse_end + diffuse_smooth, distance);
	vec4 diffuse = vec4(diffuseColor.rgb, diffuseColor.a * diff_alpha);

	float outl_alpha = smoothstep(outline_end - outline_smooth, outline_end + outline_smooth, distance);
	vec4 outline = vec4(outlineColor.rgb, outlineColor.a * outl_alpha);

	gl_FragColor = (diffuse * diff_alpha) + (outline * outl_alpha);
	//gl_FragColor = mix(diffuse, outline, outl_alpha);
	//gl_FragColor = vec4(outline.rgb, outl_alpha);

}

//varying vec2 vCoord;			// vertex texture coordinates 
//uniform sampler2D diffuseTex;	// alpha mapped texture 
//uniform vec4 diffuseColor;		// actual color for this text 
//uniform vec4 outlineColor;		// outline (or shadow) color for the text 
//
//const float smoothing = 1.0/16.0;
//const vec2 shadowOffset = vec2(-1.0/128.0);
////const vec4 glowColor = vec4(vec3(0.1), 1.0);
//const float glowMin = 0.2;
//const float glowMax = 0.8;
//// drop shadow computed in fragment shader
//void main(void)
//{
//	float dst = texture2D(diffuseTex, vCoord).r;
//	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, dst);
//	float glowDst = texture2D(diffuseTex, vCoord + shadowOffset).r;
//	vec4 glow = outlineColor * smoothstep(glowMin, glowMax, glowDst);
//	float mask = 1.0-alpha;
//	vec4 base = vec4(diffuseColor.rgb, diffuseColor.a * alpha);
//	gl_FragColor = mix(base, glow, mask);
//}