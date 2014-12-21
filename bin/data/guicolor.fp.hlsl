#version 140 // OpenGL 3.1 
varying vec4 vColor; // current vertex color

void main(void) 
{ 
	gl_FragColor = vColor;
}