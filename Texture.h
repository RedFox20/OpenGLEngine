/**
 * Copyright (c) 2013 - Jorma Rebane
 * A simple generic image loading interface
 */
#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include "Image.h"

enum WrapMode {
	WRAP_CLAMP,  // clamps texture coordinates to [0.0 .. 1.0]
	WRAP_REPEAT, // repeats the texture
};


/**
 * @note An OpenGL texture container. Data copy moves ownership of the texture
 */
struct Texture
{
	unsigned glTexture; // OpenGL texture handle
	PixelFormat format; // format of texture data: rgb, rgba, mono


	/**
	 * @note Creates an empty uninitialized texture
	 */
	inline Texture() : glTexture(0), format(FMT_INVALID) {}

	
	/**
	 * @note Acquires ownership of an already initialized OpenGL texture
	 * @param glTexture OpenGL texture handle. Will be deleted when this object dies.
	 * @param pf Pixel format of the specified texture handle
	 */
	Texture(unsigned glTexture, PixelFormat pf);


	/**
	 * @note Creates a new texture by loading a texture/image file
	 * @param filename Name of the file to load
	 */
	explicit Texture(const char* filename);

	
	/**
	 * @note Initializes a new texture from existing bitmap data
	 */
	Texture(const void* data, int width, int height, PixelFormat fmt = FMT_INVALID);


	/**
	 * @note Destroys the texture if needed
	 */
	~Texture();

	
	/**
	 * @note Initialization from another texture will move the data, leaving source texture empty (!)
	 */
	Texture(Texture& texture);


	/**
	 * @note Assignment from another texture will move the texture data, leaving the param texture empty (!)
	 */
	Texture& operator=(Texture& texture);


	/**
	 * @note Initializes the texture and overwrites an existing image
	 * @param data Image data
	 * @param width Image width
	 * @param height Image height
	 * @param pf Image pixel format
	 */
	void Create(const void* data, int width, int height, PixelFormat pf = FMT_INVALID);


	/**
	 * @note Destroys the texture if it's initialized
	 */
	void Destroy();


	/**
	 * @return TRUE if the texture has been created
	 */
	inline bool IsCreated() const { return glTexture ? true : false; }

	
	/**
	 * @return Width of the texture in pixels
	 */
	int Width() const;


	/**
	 * @return Height of the texture in pixels
	 */
	int Height() const;


	/**
	 * @note Sets the texture wrapping mode (clamp or repeat)
	 * @param wrapMode Wrapping mode to specify
	 */
	void SetWrapMode(WrapMode wrapMode);


	/**
	 * @note Copies this texture data into destination buffer
	 * @note Destination buffer is assumed to be big enough
	 * @param dst Destination buffer to copy data to
	 */
	void CopyData(void* dst) const;


	/**
	 * @note Creates a copy of this texture data and returns it in a malloc()'d buffer
	 */
	byte* CopyData() const;


	/**
	 * @return Size of the Texture data in bytes. Use in conjuction with CopyTo().
	 */
	int DataSize() const;


	/**
	 * @return Number of channels in the texture
	 */
	int Channels() const;


	/**
	 * @note Saves this texture into a file. The file is created or overwritten.
	 * @param filename Name of the file to save to.
	 * @param imgfmt File format to use when saving.
	 */
	bool SaveToFile(const char* filename, ImageFileFormat imgfmt = IMAGE_AUTO);
};


#endif // TEXTURE_H
