/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "FrameBuffer.h"
#include "GL\glew.h"
#include "GL\glut.h"


	// destroys the frame and depth buffer
	void FrameBuffer::Destroy()
	{
		if(glDepthBuffer)
			glDeleteRenderbuffers(1, &glDepthBuffer), glDepthBuffer = 0;
		if(glFrameBufferTexture)
			glDeleteTextures(1, &glFrameBufferTexture), glFrameBufferTexture = 0;
		if(glFrameBuffer)
			glDeleteFramebuffers(1, &glFrameBuffer), glFrameBuffer = 0;
	}

	// resizes or creates the framebuffer texture
	void FrameBuffer::Resize(size_t width, size_t height)
	{
		this->width = width;
		this->height = height;
		if(!glFrameBuffer) // first init
		{
			// texture
			glGenTextures(1, &glFrameBufferTexture);
			int oldTex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);
			glBindTexture(GL_TEXTURE_2D, glFrameBufferTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glBindTexture(GL_TEXTURE_2D, oldTex);

			// depth buffer
			glGenRenderbuffers(1, &glDepthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, glDepthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			// framebuffer
			glGenFramebuffers(1, &glFrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, glFrameBuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glFrameBufferTexture, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, glDepthBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}

		// resize existing
		int oldTex; glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);
		glBindTexture(GL_TEXTURE_2D, glFrameBufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, oldTex);

		glBindRenderbuffer(GL_RENDERBUFFER, glDepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	// binds this framebuffer for the rendering stage
	void FrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, glFrameBuffer);
	}

	// unbinds this framebuffer from the rendering stage
	void FrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}