/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#pragma once
#ifndef FREETYPE_CPP_H
#define FREETYPE_CPP_H

#include <vector>
#include "IShaderProgram.h"

namespace freetype
{
	// A single Glyph contains essential information of a single character in the GlyphAtlas:
	// Its unicode codepoint, width, height, horizontal advance/xy bearing, vertical advance/xy bearing, textureY
	//
	// Horizontal layout:
	//           
	//         y ^    _width_  
	//           |   |       |      ^ hBearingY
	// hBearingX |-->|       |      |
	//           |   |       |      |
	//           |   |       |      |
	//           O---|-------|------O-->
	//           |   |_______|      |   x
	//           | _ _hAdvance_ _ _ |
	//
	// Vertical layout:
	//
	//          vBearingX|
	//               <---|              x
	//           --------O------------->
	//                   |     |vBear|ngY
	//                ___|___  V     |
	//         h ^   |   |   |       |
	//         e |   |   |   |       |
	//         i |   |   |   |       |
	//         g |   |   |   |       |
	//         h |   |   |   |    vAdvance 
	//         t V   |___|___|       |
	//                   |           V
	//                   O - - - - - -
	//                   |
	//                 y V 
	struct Glyph
	{
		ushort code; 	// unicode value
		ushort index;	// freetype glyph index

		byte width;		// width of the glyph in pixels
		byte height;	// height of the glyph in pixels
		byte advance;	// number of pixels to advance on x axis
		char bearingX;	// x offset of top-left corner from x axis
		
		char bearingY;	// y offset of top-left corner from y axis
		byte textureX;	// x pixel coord of the bitmap's bottom-left corner
		ushort textureY;// y pixel coord of the bitmap's bottom-left corner 
	};

	enum FontStyle {
		FONT_PLAIN = 0,		// plain font, no stroke, no shadow
		FONT_SHADOW = 1,	// font with an additional shadow, no stroke
		FONT_OUTLINE = 2,	// font with just an outline (empty inside), no shadow
		FONT_STROKE = 4,	// font glyph with an additional stroke, no shadow
		FONT_BOLD = 8,		// font is drawn as bold
	};


	static const size_t DefaultDPI = 96;


	// forward declr:
	struct FontFace;		// a truetype font description
	struct Font;			// a sized font
	struct Text;			// base interface for text
	struct VertexText;		// type of vertex used for text glyphs



	/**
	 * A fontface describes a single truetype font file
	 * The fontface contains 
	 */
	struct FontFace
	{
		std::string fontfamily; // font family id
		byte* data;				// font data, will be freed when font is destroyed
		void* ftFace;			// freetype fontface

		inline FontFace() : data(0), ftFace(0) {}
		inline FontFace(const char* fontFile) : data(0), ftFace(0) { CreateFromFile(fontFile); }
		inline ~FontFace() { Destroy(); }

		/**
		 * Loads a specific TrueType .TTF font file and generates a FreeType face out of it
		 * @param fontFile Name of the font file to load
		 */
		bool CreateFromFile(const char* fontFile);

		/**
		 * Loads a TrueType font data and generates a FreeType face out of it
		 * Warning (!) fontData will be freed when font is destroyed
		 * @param fontData Raw font file data
		 * @param dataLength Raw font file data length (in bytes)
		 */
		bool Create(const byte* fontData, size_t dataLength);

		/**
		 * Destroys a created FontFace
		 * @param freeData Set TRUE if you want data to be freed upon face destruction
		 * You would set this false if you manage the data outside of the FontFace class.
		 */
		void Destroy(bool freeData = true);

		/** @return TRUE if the FontFace has been created */
		inline bool IsCreated() const { return ftFace ? true : false; }

		/**
		 * Creates a Font based on the provided font height
		 * @param outlineParam Depends on the specified FontStyle
		 *				FONT_PLAIN - param is ignored
		 *				FONT_SHADOW - param is the shadow offset in pixels (int)
		 *				FONT_OUTLINE - param is the outline width in sub-pixels (float)
		 *				FONT_STROKE - param is the stroke width in sub-pixels (float)
		 */
		Font* NewFont(unsigned fontHeight, FontStyle style = FONT_PLAIN, float outlineOffset = 1.0f, int dpi = DefaultDPI);
	};


	/**
	 * A Font describes a sized and styled view of a FontFace
	 * Each Font contains a GlyphAtlas
	 */
	struct Font
	{
		// A temporary buffer image for blitting the GlyphAtlas texture
		struct BufferImage
		{
			struct Pixel { byte r; };
			struct RGPixel { byte r, g; };
			Pixel* Data;
			int Width, Height;
			int Channels;

			// creates an uninitialized buffer
			BufferImage();

			// creates a new buffer
			BufferImage(int w, int h, int channels);

			// creates a buffer from an existing texture
			// new size must be >= than original size
			BufferImage(Texture& srcTex, int newWidth, int newHeight);

			~BufferImage(); // cleanup when done

			// initializes image to the specified format
			void InitImage(int w, int h, int channels);


			// sets the subimage in the main channel !!performs ROW FLIPPING!!
			void SetSubImage(int x, int y, int srcW, int srcH, byte* src);

			// masks a subimage to the background channel, each pixel is masked
			// against the main channel and reduced accordingly !!performs ROW FLIPPING!!
			void MaskSubImage(int x, int y, int srcW, int srcH, byte* src);

			// masks a subimage to the main channel !!performs ROW FLIPPING!!
			void MaskSubImage0(int x, int y, int srcW, int srcH, byte* src);

			// copies source BufferImage into the specified coordinates
			void CopySubImage(int x, int y, const BufferImage& src);
		};

		/**
		 * A glyphatlas contains a font texture with a sorted list of glyphs
		 */
		struct GlyphAtlas
		{
			Texture mTexture;	// texture that contains all the rendered glyphs
			std::vector<Glyph> mGlyphs;  // all glyphs of this fontmap
			Font* font;			// ptr to font that owns this atlas
			void* ftStroker;	// freetype stroker definition
			FontStyle mStyle;	// font style

			ushort mPackX;		// current X position for glyph packing
			ushort mPackY;		// current Y position for glyph packing
			ushort mPackHeight;	// current height of glyph packing
			ushort mWidth;		// current width of the virtual texture
			ushort mHeight;		// current height of the virtual texture
			ushort mPadding;	// outline or shadow padding, depending on FONTSTYLE

			// max texture size allowed by opengl
			static const size_t MAX_TEXTURE_SIZE = 16384;

			inline GlyphAtlas() : font(0), ftStroker(0) {}
			inline ~GlyphAtlas() { Destroy(); }
		
			/** Creates the default subset of the atlas and initializes state variables */
			void Create(Font* font, int fontHeight, FontStyle style, float outlineParam);

			/** Creates a new glyph. Modifies: packX, packHeight, width, height */
			void CreateGlyph(Glyph& out, BufferImage& img, uint charcode, uint glyphIndex);

			/** Renders a glyph into the bufferimage */
			void RenderGlyph(BufferImage& dst, const Glyph& g, const BufferImage& img);

			/**
			 * Generates new glyphs based on the input string
			 * @note This function filters out glyphs that have been generated
			 *       and only generates glyphs for missing glyphs
			 *       Therefore you can call this anytime without much fear
			 */
			void GenerateGlyphs(const wchar_t* str, size_t len);

			/** destroys the entire atlas */
			void Destroy();
		
			/** Selects the correct size from the freetype face */
			void SelectFaceSize();



			/** Emplaces a new glyph for the specified char into the glyphatlas (binary sort insert) */
			Glyph* Emplace(wchar_t ch);
		
			/**
			 * retrieves a pointer to the specified codepoint in this glyphatlas
			 * if a glyph can't be generated at all, the default glyph is returned
			 * if a glyph hasn't been generated yet (but possible), NULL is returned
			 */
			const Glyph* Get(uint codepoint) const;

			/** returns the number of glyphs in this glyphmap */
			inline unsigned Size()   const { return mGlyphs.size();    }
			inline unsigned Width()  const { return mTexture.Width();  }
			inline unsigned Height() const { return mTexture.Height(); }

			/** @return TRUE if atlas texture has been created */
			bool IsCreated() const { return mTexture.IsCreated(); }
		};


		GlyphAtlas atlas;	// container atlas of the glyphs
		FontFace* face;		// TrueType fontface of this font
		void* ftSize;		// freetype font size definition held by each Font
		ushort height;		// font height in pixels
		ushort dpi;			// font dpi
		bool   is_sdf;      // is this a signed distance field font?

		inline Font() : face(0), ftSize(0), is_sdf(false) {}
		inline ~Font() { Destroy(); }
		
		/** @return Specified height of the font in pixels */
		inline int FontHeight() const { return height; }
		/** @return Distance in pixels to a newline */
		int LineHeight() const;
		/** @return Specified DPI of the font, default is 96 */
		inline int FontDPI() const { return dpi; }


		/**
		 * Creates a Font based on the provided FontFace
		 * @param outlineParam Depends on the specified FontStyle
		 *				FONT_PLAIN - param is ignored
		 *				FONT_SHADOW - param is the shadow offset in pixels (int)
		 *				FONT_OUTLINE - param is the outline width in sub-pixels (float)
		 *				FONT_STROKE - param is the stroke width in sub-pixels (float)
		 */
		bool Create(FontFace* face, unsigned fontHeight, FontStyle style = FONT_PLAIN, float outlineOffset = 1.0f, int dpi = DefaultDPI);

		/** Destroys this Font and its atlas */
		void Destroy();

		/** @return TRUE if atlas is created */
		bool IsCreated() const { return atlas.IsCreated(); }




		/**
		 * Creates a new text object. Must be deleted by the user.
		 * @param fmt Format string to feed to sprintf
		 * @param ... Variable list of arguments depending on fmt
		 * @return A new Text object
		 */
		Text* CreateTextF(const wchar_t* fmt, ...);
		/**
		 * Creates a new text object. Must be deleted by the user.
		 * @param fmt Format string to feed to sprintf
		 * @param ap Variable list argument pointer
		 * @return A new Text object
		 */
		Text* CreateTextV(const wchar_t* fmt, va_list ap);
		/**
		 * Creates a new text object. Must be deleted by the user.
		 * @param str String to create text from
		 * @return A new Text object
		 */
		Text* CreateText(const wchar_t* str);
		/**
		 * Creates a new text object. Must be deleted by the user.
		 * @param str String to create text object from
		 * @param len Length of the string
		 * @return A new Text object
		 */
		Text* CreateText(const wchar_t* str, size_t len);

		/**
		 * @brief Generates a block of glyph vertices based on input string
		 * @param outVerts Out destination buffer; sizeof = sizeof(VertexText) * 6 * len
		 * @param outSize Out bounding box size of the glyph block
		 * @param str String to generate vertices from
		 * @param len Length of the string
		 * @return Number of vertices actually generated (usually != len)
		 */
		size_t GenBlock(VertexText* outVerts, Vector2& outSize, const wchar_t* str, size_t len);

	};


	/** @note Describes a simple 4D vertex */
	struct VertexText
	{
		float x, y, u, v; // attribute vertex.xyuv
		static const VertexDescr* GetVertexDescr() {
			static VertexDescr descr = { 1, sizeof(VertexText), {4} };
			return &descr;
		}
	};



	/**
	 * Base text interface
	 */
	struct Text
	{
		VertexBuffer vb;// vertex data of the text
		Vector2 size;	// size of the text [wh]
		Font* font;		// font for rendering this text

		Text() : font(NULL) {}

		/** @return Type of this text object (Static/Dynamic) */
		BufferType Type() const;
		/** @return Number of glyphs in this text */
		size_t Length() const;
		/** @return Specified glyphs XY offset */
		Vector2 GlyphXY(size_t i) const;

		/** @return TRUE if this text has been initialized */
		inline bool IsCreated() const { return font != NULL; }
		/** @return Bounding width of the text */
		inline float Width() const { return size.x; }
		/** @return Bounding width of the text */
		inline float Height() const { return size.y; }
		/** @return Bounding size of the text */
		inline Vector2 Size() const { return size; }

		/**
		 * Draws this text on the screen
		 * @param mvp The model-view-projection matrix to translate the text position+scale+rotation
		 * @param color Color of the text
		 * @param outline Color of the stroke or shadow
		 */
		void Draw(const Matrix4& mvp, const Vector4& color, const Vector4& outline);

		/**
		 * Draws a sub-segment of the text on the screen
		 * @param start Start index of the sub-segment of text to render
		 * @param count Number of characters to render
		 * @param mvp The model-view-projection matrix to translate the text position+scale+rotation
		 * @param color Color of the text
		 * @param outline Color of the stroke or shadow
		 */
		void Draw(uint start, uint count, const Matrix4& mvp, const Vector4& color, const Vector4& outline);

		/**
		 * @brief Creates or Re-Creates this text object based on input string
		 * @param str String to create text from
		 * @param len Length of the string
		 */
		void Create(const wchar_t* str, size_t len);

		/**
		 * @brief Appends new text to the end of this text
		 * @param str String to create text from
		 * @param len Length of the string
		 */
		void Append(const wchar_t* str, size_t len);

		/**
		 * @brief Inserts new text inside of this text
		 * @param str String to create text from
		 * @param len Length of the string
		 */
		void Insert(int index, const wchar_t* str, size_t len);
	};

}

#endif // FREETYPE_CPP_H