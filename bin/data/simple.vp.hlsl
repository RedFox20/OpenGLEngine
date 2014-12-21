#version 140 // OpenGL 3.1

attribute vec3 position;	// vertex position
attribute vec2 coord;		// vertex texture coordinates
uniform mat4 transform;							// transformation matrix
varying vec2 vCoord;							// out vertex texture coord for frag
 
void main(void)
{
	gl_Position = transform * vec4(position, 1.0);
	vCoord = coord;
}