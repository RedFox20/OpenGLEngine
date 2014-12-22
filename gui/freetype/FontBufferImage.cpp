/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "FreeType.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

namespace freetype
{

	// creates an uninitialized buffer
	Font::BufferImage::BufferImage() : Data(0), Width(0), Height(0), Channels(0)
	{
	}

	// creates a new buffer
	Font::BufferImage::BufferImage(int w, int h, int channels) : Width(w), Height(h), Channels(channels)
	{
		size_t size = w * h * channels;
		Data = (Pixel*)malloc(size);
		memset(Data, 0, size); // all pixels to black
	}

	// creates a buffer from an existing texture, new size must be >= than original size
	Font::BufferImage::BufferImage(Texture& srcTex, int newWidth, int newHeight) : Width(newWidth), Height(newHeight)
	{
		Channels = srcTex.format == FMT_R ? 1 : 2; // get number of channels
		size_t size = newWidth * newHeight * Channels;
		Data = (Pixel*)malloc(size);
		size_t oldSize = srcTex.Width() * srcTex.Height() * Channels;
		srcTex.CopyData(Data); // copy texture data to this buffer
		memset((char*)Data + oldSize, 0, size - oldSize); // rest of the pixels to black
	}

	Font::BufferImage::~BufferImage() { free(Data); } // some cleanup



	// initializes image to the specified format
	void Font::BufferImage::InitImage(int w, int h, int channels)
	{
		Width = w;
		Height = h;
		Channels = channels;
		size_t size = w * h * channels;
		Data = (Pixel*)realloc(Data, size);
		memset(Data, 0, size); // all pixels to black
	}


	// sets the subimage in the main channel
	void Font::BufferImage::SetSubImage(int x, int y, int srcW, int srcH, byte* src) 
	{
		src += srcW * (srcH-1); // set source to the last row
		int channels = Channels;
		int stride   = Width * channels;         // line stride in bytes
		byte* dst    = (byte*)Data + x*channels + (y*stride); // set the first row
		if (Height < (y + srcH))
		{
			fprintf(stderr, "Error! It won't fit man!\n");
			srcH = Height - y; // make it fit
		}
		while (srcH) 
		{
			byte* p = (byte*)dst;
			for (int i = 0; i < srcW; ++i)
			{
				*p = src[i]; // fill the RED channel
				p += channels;
			}
			dst += stride;  // advance dst by stride
			src -= srcW; // retreat src by src width
			--srcH;
		}
	}

	// masks a subimage to the background channel, each pixel is masked
	// against the main channel and reduced accordingly
	void Font::BufferImage::MaskSubImage(int x, int y, int srcW, int srcH, byte* src)
	{
		if (Channels == 1) 
			return; // can't do anything if only 1 channel
		src += srcW * (srcH-1); // set source to the last row
		RGPixel* dst = (RGPixel*)Data + x + (y*Width); // set the first row
		if (Height < (y + srcH))
		{
			fprintf(stderr, "Error! It won't fit man!\n");
			srcH = Height - y; // make it fit
		}
		while (srcH) 
		{
			for (int i = 0; i < srcW; ++i)
			{
				// get the new value
				uint value = (uint)src[i] + (uint)dst[i].g;

				// get the max possible background value
				uint max = 255 - dst[i].r; 

				// if shadow pixel is brighter than available
				dst[i].g = value > max ? max : (byte)value;
			}
			dst += Width; // advance dst by image width
			src -= srcW; // retreat src by src width
			--srcH;
		}
	}

	// masks a subimage to the main channel !!performs ROW FLIPPING!!
	void Font::BufferImage::MaskSubImage0(int x, int y, int srcW, int srcH, byte* src)
	{
		src += srcW * (srcH-1); // set source to the last row
		int stride = Channels*Width;
		byte* dst = (byte*)Data + x*Channels + (y*stride); // set the first row
		if (Height < (y + srcH))
		{
			fprintf(stderr, "Error! It won't fit man!\n");
			srcH = Height - y; // make it fit
		}
		while (srcH) 
		{
			for (int i = 0; i < srcW; ++i)
			{
				// get the new value
				uint value = (uint)src[i] + (uint)dst[i];
				if (value > 255) value = 255;
				dst[i] = value;
			}
			dst += stride; // advance dst by image stride
			src -= srcW; // retreat src by src width
			--srcH;
		}
	}

	void Font::BufferImage::CopySubImage(int x, int y, const BufferImage& img)
	{
		int srcW = img.Width, srcH = img.Height, srcChannels = img.Channels, srcStride = srcW*srcChannels;
		const byte* src = (byte*)img.Data;
		int w = Width, h = Height, channels = Channels, stride = w*channels;
		byte* dst = (byte*)Data + x*channels + (y*stride); // set the first row

		if (h < (y + srcH))
		{
			fprintf(stderr, "Error! Source BufferImage Y destination too big\n");
			srcH = h - y; // make it fit
		}

		if (channels < srcChannels)
		{
			fprintf(stderr, "Error! Destination has less color channels than source BufferImage!\n");
			return;
		}
		else if (channels == srcChannels)
		{
			while (srcH)
			{
				memcpy(dst, src, srcStride); // copy entire image row
				dst += stride;
				src += srcStride;
				--srcH;
			}
		}
		else // channels > srcChannels => channels:2 srcChannels:1
		{
			while (srcH) 
			{
				byte* p = dst;
				for (int i = 0; i < srcStride; ++i)
				{
					*p = src[i]; // fill the RED channel ONLY
					p += channels;
				}
				dst += stride;
				src += srcStride;
				--srcH;
			}
		}
	}

	
} // namespace freetype