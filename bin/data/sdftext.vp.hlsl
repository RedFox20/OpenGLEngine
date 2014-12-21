#version 140 // OpenGL 3.1
attribute vec4 vertex;	// vertex position[xy] and texture coord[zw]
uniform mat4 transform;						// transformation matrix; also contains the depth information
uniform sampler2D diffuseTex;				// alpha mapped texture 
varying vec2 vCoord;						// out vertex texture coord for frag
varying vec2 vPixel;                        // out 1.0 / textureSize

void main(void)
{
	// we only need xy here, since the projection is trusted to be
	// orthographic. Any depth information is encoded in the transformation
	// matrix itself. This helps to minimize the bandwidth.
	gl_Position = transform * vec4(vertex.xy, 0.0, 1.0);

	// since texture coordinates are in pixel values, we'll need to
	// generate usable UV's on-the-go
	// send the texture coordinates to the fragment shader:
	vec2 texSize = textureSize(diffuseTex, 0);
	vPixel = 1.0 / texSize;
	vCoord = vertex.zw / texSize; 
}