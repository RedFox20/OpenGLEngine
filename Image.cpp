/** 
 * Copyright (c) 2013 - Jorma Rebane 
 */
#include "Image.h"
#include <stdio.h>
#include <string.h>
#include "GL\glew.h"


	/**
	 * @note Creates a new Image by loading the specified file
	 * @param filename Name of the file to load
	 */
	Image::Image(const char* filename) : format(FMT_INVALID), data(0), width(0), height(0)
	{
		LoadFile(filename);
	}


	/**
	 * @note Creates a new Image by moving image data from the specified image
	 * @param img Image to move data from. This image will be reset to default uninitialized image.
	 */
	Image::Image(Image& img)
	{
		if (this != &img) // Image img = img; // sigh...
			format = img.format, data = img.data, width = img.width, height = img.height;
		img.format = FMT_INVALID, img.data = 0, img.width = 0, img.height = 0;
	}

	/**
	 * @note Destroys the Image if needed
	 */
	Image::~Image()
	{
		if(data)
			free(data);
	}


	/**
	 * @note Assignment from another Image will move the image data, leaving the param image empty (!)
	 */
	Image& Image::operator=(Image& img)
	{
		if (this != &img)
		{
			format = img.format, data = img.data, width = img.width, height = img.height;
			img.format = FMT_INVALID, img.data = 0, img.width = 0, img.height = 0;
		}
		return *this;
	}


	/**
	 * @note Destroys the Image by deallocating the data with free()
	 */
	void Image::Destroy()
	{
		if(data)
		{
			free(data);
			data = 0;
			width = 0;
			height = 0;
		}
	}


	/**
	 * @note Saves this Image to the destination file
	 * @param filename Destination file name to create or overwrite
	 * @param imgfmt Image file format
	 * @return TRUE if save was successful
	 */
	bool Image::SaveToFile(const char* filename, ImageFileFormat imgfmt)
	{
		return SaveToFile(filename, imgfmt, data, width, height, format);
	}


	/**
	 * @return Number of channels specified by a PixelFormat
	 */
	static int PixelFormatChannels(PixelFormat pf)
	{
		switch(pf) {
		default: return 0;
		case FMT_R: return 1;
		case FMT_RG: return 2;
		case FMT_RGB: case FMT_BGR: return 3;
		case FMT_RGBA: case FMT_BGRA: return 4;
		}
	}
	
	/**
	 * @return Number of channels in the image
	 */
	int Image::Channels() const
	{
		return PixelFormatChannels(format);
	}





	//// ---- BMP format structures ---- ////
#pragma pack(push)
#pragma pack(1) // make sure no struct alignment packing is made
	struct CIEXYZTRIPLE { struct CIEXYZ { long x, y, z; } r, g, b; };
	struct RGBQUAD { unsigned char b, g, r, x; };
	struct BitmapFileHeader { ushort Type; unsigned Size, Reserved, OffBits; };
	struct BitmapInfoHeader { unsigned Size; int Width, Height; ushort Planes, BitCount; unsigned Compression, SizeImage; unsigned XPelsPerMeter, YPelsPerMeter; unsigned ClrUsed, ClrImportant; };
	struct BitmapV5InfoHeader { BitmapInfoHeader BIH; unsigned RedMask, GreenMask, BlueMask, AlphaMask, CSType; CIEXYZTRIPLE EndPoints; unsigned GammaRed, GammaGreen, GammaBlue, Intent, ProfileData, ProfileSize, Reserved; };
#pragma pack(pop)
	static bool LoadBMP(FILE* file, byte** data, int* width, int* height, PixelFormat* pf);
	static bool SaveBMP(FILE* file, const byte* data, int width, int height, PixelFormat pf);






	/**
	 * @note Creates a new Image by loading the specified file
	 * @param filename Image file to load
	 */
	bool Image::LoadFile(const char* filename)
	{
		if(data) Destroy(); // destroy existing data

		FILE* file = fopen(filename, "rb");
		if(!file) return false; // file does not exist

		union UniHeader { BitmapFileHeader bmp; } Header;

		bool result = false;
		fread(&Header, sizeof(Header), 1, file);
		if(Header.bmp.Type == 19778) // it's a BMP
			result = LoadBMP(file, &data, &width, &height, &format);

		fclose(file);
		return result;
	}

	/**
	 * @note Autodetects image file format based on filename
	 */
	ImageFileFormat AutoFileFormat(const char* filename)
	{
		int fmt = *(int*)(filename + strlen(filename) - 4);
		switch(fmt) 
		{
			case 'pmb.'/*.bmp*/: return IMAGE_BMP;
			default: return IMAGE_AUTO; // unsupported format
		}
	}


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
	bool Image::SaveToFile(const char* filename, ImageFileFormat imgfmt, const byte* data, int width, int height, PixelFormat pf)
	{
		if(imgfmt == IMAGE_AUTO)
			imgfmt = AutoFileFormat(filename); // try to figure out the image format
		if(imgfmt == IMAGE_AUTO) // re-check
			return false; // we don't know how to save the image

		FILE* file = fopen(filename, "wb");
		if(!file) return false; // file already opened

		bool result = false;
		switch(imgfmt) 
		{
			case IMAGE_BMP: result = SaveBMP(file, data, width, height, pf); break;
		}

		fclose(file);
		return result;
	}









	//// ---- BMP FORMAT IMPL. ---- ////


	static bool LoadBMP(FILE* file, byte** data, int* width, int* height, PixelFormat* pf)
	{
		BitmapFileHeader bmh;
		BitmapInfoHeader bmi;
		fseek(file, 0, SEEK_SET); // reset any seek
		fread(&bmh, sizeof(bmh), 1, file); // read BitmapFileHeader for OffBits
		fread(&bmi, sizeof(bmi), 1, file); // read BitmapInfoHeader for size info
		switch(bmi.BitCount >> 3) // number of channels?
		{
			default: return false; // failed - unsupported data format
			case 1: *pf = FMT_R; break;
			case 3: *pf = FMT_BGR; break;
			case 4: *pf = FMT_BGRA; break;
		}
		*data = (byte*)malloc(bmi.SizeImage); // allocate enough for the entire image
		fseek(file, bmh.OffBits, SEEK_SET); // seek to start of image data
		fread(*data, bmi.SizeImage, 1, file); // read the image data
		*width = bmi.Width;
		*height = bmi.Height;
		return true;
	}
	static bool SaveBMP(FILE* file, const byte* data, int width, int height, PixelFormat pf)
	{
		int paddedSize;
		byte* paddedData = NULL;
		int channels = PixelFormatChannels(pf);
		int rowSize = channels * width;
		if ((rowSize & 3) != 0) // do we need to align the rows? (BMP requirement)
		{
			int paddedRowSize = rowSize + 4 - (rowSize & 3);
			paddedSize = paddedRowSize * height;
			paddedData = (byte*)malloc(paddedSize);

			byte* dst = paddedData;
			const byte* src = data;
			for (int i = 0; i < height; i++) // for each row in the image
			{
				// just copy the padded amount straight away
				// the padded data will be somewhat garbage, since it's taken from the next row
				memcpy(dst, src, paddedRowSize);
				dst += paddedRowSize; // advance dst by padded rows
				src += rowSize; // advance src by raw data size
			}
			data = paddedData; // just overwrite the original data pointer (since we already made a copy)
		}
		else // data is already aligned (yay!)
		{
			paddedSize = rowSize * height;
		}

		// 8-bit data requires a color table (!)
		if (channels == 1)
		{
			RGBQUAD colors[256]; // we need to create a color table for an 8-bit image:
			for(int i = 0; i < 256; i += 4) { // unrolled for win
				colors[i+0].b = i+0, colors[i+0].g = i+0, colors[i+0].r = i+0, colors[i+0].x = 0;
				colors[i+1].b = i+1, colors[i+1].g = i+1, colors[i+1].r = i+1, colors[i+1].x = 0;
				colors[i+2].b = i+2, colors[i+2].g = i+2, colors[i+2].r = i+2, colors[i+2].x = 0;
				colors[i+3].b = i+3, colors[i+3].g = i+3, colors[i+3].r = i+3, colors[i+3].x = 0;
			}
			const size_t HSize = sizeof(BitmapFileHeader) + sizeof(BitmapV5InfoHeader) + sizeof(colors);
			BitmapFileHeader bmfV5 = { 19778, HSize + paddedSize, 0, HSize,	};
			BitmapV5InfoHeader bmiV5 = { {
					sizeof(BitmapV5InfoHeader), // size of the V5 struct
					width, height,		// height of the image - negative for top-down bitmap
					1, 8, 0,					// 1 plane, 8bpp, BI_RGB(uncompressed)
					paddedSize,					// image data size
					3780, 3780,					// X/YPelsPerMeter
					256, 0						// 256 colors in the color table, all colors
				}, 0, // set rest to 0
			};
			bmiV5.CSType = 'sRGB';	// sRGB colorspace
			bmiV5.Intent = 8;		// LCS_GM_ABS_COLORIMETRIC - use nearest palette match

			fwrite(&bmfV5, sizeof(bmfV5), 1, file);
			fwrite(&bmiV5, sizeof(bmiV5), 1, file);
			fwrite(&colors, sizeof(colors), 1, file);
		}
		else
		{
			BitmapFileHeader bmf = { 19778, 54 + paddedSize, 0, 54, };
			BitmapInfoHeader bmi = {
				sizeof(BitmapInfoHeader),	// size of this struct
				width, height,		// height of the image - negative for top-down bitmap
				1, channels*8, 0,			// 1 plane, number of bits: 8/24/32, BI_RGB(uncompressed)
				paddedSize,					// size of image
				3780, 3780,					// X/YPelsPerMeter
				0, 0,						// No colortable
			};
			fwrite(&bmf, sizeof(bmf), 1, file);
			fwrite(&bmi, sizeof(bmi), 1, file);
		}
		fwrite(data, paddedSize, 1, file);

		if (paddedData) free(paddedData); // clean up if we used paddedData
		return true;
	}
