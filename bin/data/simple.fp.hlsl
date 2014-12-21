#version 140 // OpenGL 3.1
 
varying vec2 vCoord;	// vertex texture coordinates
uniform sampler2D diffuseTex;

void main(void)
{
	gl_FragColor = texture2D(diffuseTex, vCoord);
}