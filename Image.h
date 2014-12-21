/** 
 * Copyright (c) 2013 - Jorma Rebane 
 */
#pragma once
#ifndef IMAGE_H
#define IMAGE_H

#include "Basetypes.h"

enum ImageFileFormat 
{
	IMAGE_AUTO, // automatically detect image format from file extension
	IMAGE_BMP,
};


enum PixelFormat 
{
	FMT_INVALID,
	FMT_R,
	FMT_RG,
	FMT_RGB,
	FMT_BGR,
	FMT_RGBA,
	FMT_BGRA,
};


/** 
 * @note A simple Image structure stored in CPU RAM. Also used for loading/saving image data
 */
struct Image
{
	byte* data; // image data
	int width;  // image width
	int height; // image height
	PixelFormat format; // data type


	/**
	 * @note Creates an empty uninitialized Image
	 */
	inline Image() : data(0), width(0), height(0), format(FMT_INVALID) {}


	/**
	 * @note Creates a new Image by taking ownership of a raw data pointer
	 * @param data Pointer to data to take ownership of. Will be freed when Image is destroyed.
	 * @param width Width of the image
	 * @param height Height of the image
	 * @param it Image type
	 */
	inline Image(byte* data, int width, int height, PixelFormat pf) : data(data), width(width), height(height), format(pf) {}
	

	/**
	 * @note Creates a new Image by loading the specified file
	 * @param filename Name of the file to load
	 */
	explicit Image(const char* filename);


	/**
	 * @note Creates a new Image by moving image data from the specified image
	 * @param img Image to move data from. This image will be reset to default uninitialized image.
	 */
	Image(Image& img);
	

	/**
	 * @note Destroys the Image if needed
	 */
	~Image();


	/**
	 * @note Assignment from another Image will move the image data, leaving the param image empty (!)
	 */
	Image& operator=(Image& img);


	/**
	 * @note Creates a new Image by loading the specified file
	 * @param filename Image file to load
	 */
	bool LoadFile(const char* filename);
	

	/**
	 * @note Destroys the Image by deallocating the data with free()
	 */
	void Destroy();


	/**
	 * @note Saves this Image to the destination file
	 * @param filename Destination file name to create or overwrite
	 * @param imgfmt Image file format
	 * @return TRUE if save was successful
	 */
	bool SaveToFile(const char* filename, ImageFileFormat imgfmt = IMAGE_AUTO);


	/**
	 * @note Save image data to the destination file
	 * @param filename Destination file name to create or overwrite
	 * @param imgfmt Image file format
	 * @param data Image data
	 * @param width Image width
	 * @param height Image height
	 * @param pf Pixel format
	 * @return TRUE if save was successful
	 */
	static bool SaveToFile(const char* filename, ImageFileFormat imgfmt, const byte* data, int width, int height, PixelFormat pf);


	/**
	 * @return Pixel format of the image
	 */
	inline PixelFormat Format() const { return format; }
	

	/**
	 * @return Pointer to image data
	 */
	inline const byte* Data() const { return data; }


	/**
	 * @return Image width
	 */
	inline int Width() const { return width; }


	/**
	 * @return Image height
	 */
	inline int Height() const { return height; }


	/**
	 * @return Size of image data in bytes
	 */
	inline int DataSize() const { return Channels() * width * height; }


	/**
	 * @return Number of channels in the image
	 */
	int Channels() const;


	/**
	 * @return TRUE if image format is BGR instead of normal RGB
	 */
	inline bool IsBGR() const { return format == FMT_BGR || format == FMT_BGRA; }


	/**
	 * @return TRUE if Image has been initialized / loaded
	 */
	inline bool IsCreated() const { return data ? true : false; }
};


/**
 * VS2012 natvis debug bitmap, can be viewed with VS Image Watch in debug mode
 * @note Requires DebugBitmap.natvis Visualization file (!)
 */
struct DebugBitmap
{
	unsigned int width;
	unsigned int height;
	unsigned int nchannels;
	unsigned char* data;

	inline DebugBitmap(const void* src, int width, int height, int nchannels) 
		: width(width), height(height), nchannels(nchannels)
	{
		size_t sz = width * height * nchannels;
		data = (unsigned char*)malloc(sz);
		memcpy(data, src, sz);
	}
	inline ~DebugBitmap()
	{
		free(data);
	}
};


#endif // IMAGE_H