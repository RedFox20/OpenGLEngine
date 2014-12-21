#version 140 // OpenGL 3.1
attribute vec2 vertex;	// vertex position[xy] and packedRGBA[z]
attribute vec4 color;
uniform mat4 transform;						// transformation matrix; also contains the depth information
varying vec4 vColor;						// current vertex color

void main(void)
{
	gl_Position = transform * vec4(vertex.xy, 0.0, 1.0);
	vColor = color;

	//// unpackRGBA: vertex.z
	//vec4 result = fract(vertex.z * vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0));
	//result -= result.xxyz * vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
	//vColor = result;
}