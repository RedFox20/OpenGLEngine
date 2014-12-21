#include "Texture.h"

#include "GL\glew.h"
#include <cstdio> // fopen
#include <cstring> // strrchr
#include <malloc.h> // malloc/free

	static unsigned glCurrentTexture = 0;
	#define BindIfNeeded() if(glCurrentTexture != glTexture) { glCurrentTexture = glTexture; glBindTexture(GL_TEXTURE_2D, glTexture); }


	struct FormatDescriptor { ushort channels, bmpFormat, glFormat; };
	static FormatDescriptor GetFormatDescriptor(PixelFormat pf)
	{
		FormatDescriptor d = { 0, 0, 0 };
		switch(pf) {
		case FMT_R: d.channels = 1, d.bmpFormat = GL_RED, d.glFormat = GL_R8; break;
		case FMT_RG: d.channels = 2, d.bmpFormat = GL_RG, d.glFormat = GL_RG8; break;
		case FMT_RGB: d.channels = 3, d.bmpFormat = GL_RGB, d.glFormat = GL_RGB8; break;
		case FMT_BGR: d.channels = 3, d.bmpFormat = GL_BGR, d.glFormat = GL_RGB8; break;
		case FMT_RGBA: d.channels = 4, d.bmpFormat = GL_RGBA, d.glFormat = GL_RGBA8; break;
		case FMT_BGRA: d.channels = 4, d.bmpFormat = GL_BGRA, d.glFormat = GL_RGBA8; break;
		}
		return d;
	}




	/**
	 * @note Acquires ownership of an already initialized OpenGL texture
	 * @param glTexture OpenGL texture handle. Will be deleted when this object dies.
	 * @param pf Pixel format of the specified texture handle
	 */
	Texture::Texture(unsigned glTexture, PixelFormat pf) : glTexture(glTexture), format(pf)
	{
	}


	/**
	 * @note Creates a new texture by loading a texture/image file
	 * @param filename Name of the file to load
	 */
	Texture::Texture(const char* filename) : glTexture(0), format(FMT_INVALID)
	{
		Image img(filename);
		Create(img.data, img.width, img.height, img.format);
	}


	/**
	 * @note Initializes a new texture from existing bitmap data
	 */
	Texture::Texture(const void* data, int width, int height, PixelFormat pf) : glTexture(0), format(FMT_INVALID)
	{
		Create(data, width, height, pf);
	}


	/**
	 * @note Destroys the texture if needed
	 */
	Texture::~Texture()
	{
		if(glTexture)
			glDeleteTextures(1, &glTexture);
	}


	/**
	 * @note Initialization from another texture will move the data, leaving source texture empty (!)
	 */
	Texture::Texture(Texture& rhs)
	{
		if(this == &rhs) // Texture t = t; // sigh...
			glTexture = rhs.glTexture, format = rhs.format;
		rhs.glTexture = 0, rhs.format = FMT_INVALID;
	}


	/**
	 * @note Assignment from another texture will move the texture data, leaving the param texture empty (!)
	 */
	Texture& Texture::operator=(Texture& rhs)
	{
		if(this != &rhs) // Texture t; t = t; // damn.
		{
			glTexture = rhs.glTexture, format = rhs.format;
			rhs.glTexture = 0, rhs.format = FMT_INVALID;
		}
		return *this;
	}


	/**
	 * @note Initializes the texture and overwrites an existing image
	 * @param data Image data
	 * @param width Image width
	 * @param height Image height
	 * @param pf Image pixel format
	 */
	void Texture::Create(const void* data, int width, int height, PixelFormat pf)
	{
		FormatDescriptor d = GetFormatDescriptor(pf);
		if (!glTexture) glGenTextures(1, &glTexture);
		BindIfNeeded();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, d.glFormat, width, height, 0, d.bmpFormat, GL_UNSIGNED_BYTE, data);
		this->format = pf;
	}


	/**
	 * @note Destroys the texture if it's initialized
	 */
	void Texture::Destroy()
	{
		if(glTexture)
		{
			glDeleteTextures(1, &glTexture);
			glTexture = 0;
			format = FMT_INVALID;
		}
	}


	/**
	 * @return Width of the texture in pixels
	 */
	int Texture::Width() const
	{
		BindIfNeeded();
		int value;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &value);
		return value;
	}


	/**
	 * @return Height of the texture in pixels
	 */
	int Texture::Height() const
	{
		BindIfNeeded();
		int value;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &value);
		return value;
	}


	/**
	 * @note Sets the texture wrapping mode (clamp or repeat)
	 * @param wrapMode Wrapping mode to specify
	 */
	void Texture::SetWrapMode(WrapMode wrapMode)
	{
		GLenum mode;
		switch(wrapMode) {
		case WRAP_CLAMP: mode = GL_CLAMP_TO_EDGE; break;
		case WRAP_REPEAT: mode = GL_REPEAT; break;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
	}


	/**
	 * @note Copies this texture data into destination buffer
	 * @note Destination buffer is assumed to be big enough
	 * @param dst Destination buffer to copy data to
	 */
	void Texture::CopyData(void* dst) const
	{
		if(!glTexture) return; // nothing to do here
		BindIfNeeded();
		glGetTexImage(GL_TEXTURE_2D, 0, GetFormatDescriptor(format).glFormat, GL_UNSIGNED_BYTE, dst);
	}


	/**
	 * @note Creates a copy of this texture data and returns it in a malloc()'d buffer
	 */
	byte* Texture::CopyData() const
	{
		byte* data = (byte*)malloc(DataSize());
		CopyData(data);
		return data;
	}


	/**
	 * @return Size of the Texture data in bytes. Use in conjuction with CopyTo().
	 */
	int Texture::DataSize() const
	{
		return Width() * Height() * GetFormatDescriptor(format).channels;
	}


	/**
	 * @return Number of channels in the texture
	 */
	int Texture::Channels() const
	{
		return GetFormatDescriptor(format).channels;
	}


	/**
	 * @note Saves this texture into a file. The file is created or overwritten.
	 * @param filename Name of the file to save to.
	 * @param imgfmt File format to use when saving.
	 */
	bool Texture::SaveToFile(const char* filename, ImageFileFormat imgfmt)
	{
		FormatDescriptor d = GetFormatDescriptor(format);
		ushort width = Width(), height = Height();
		int dataSize = d.channels * width * height;
		byte* data = (byte*)malloc(dataSize); // this data is not padded (!)
		
		BindIfNeeded();
		glGetTexImage(GL_TEXTURE_2D, 0, d.glFormat, GL_UNSIGNED_BYTE, data);
		bool result = Image::SaveToFile(filename, imgfmt, data, width, height, format);

		free(data);
		return result;
	}


