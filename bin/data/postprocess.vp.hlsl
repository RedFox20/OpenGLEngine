#version 140 // OpenGL 3.1 

layout(location = 0) attribute vec4 vertex; // pos xy, tex st
varying vec2 vCoord;						// out vertex coord

void main()
{
	gl_Position = vec4(vertex.xy, 0.0f, 1.0f);
	vCoord = vertex.zw;
}