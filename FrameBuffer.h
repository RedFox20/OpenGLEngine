/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

struct FrameBuffer
{
	unsigned glFrameBufferTexture;
	unsigned glFrameBuffer;
	unsigned glDepthBuffer;
	unsigned short width;
	unsigned short height;

	inline FrameBuffer() : glFrameBufferTexture(0), glFrameBuffer(0), glDepthBuffer(0), width(0), height(0) {}
	inline ~FrameBuffer() { Destroy(); }

	// destroys the frame and depth buffer
	void Destroy();

	// resizes or creates the framebuffer texture
	void Resize(size_t width, size_t height);

	// binds this framebuffer for the rendering stage
	void Bind();

	// unbinds this framebuffer from the rendering stage
	void Unbind();

	size_t Width() const;
	size_t Height() const;

};


#endif // FRAMEBUFFER_H