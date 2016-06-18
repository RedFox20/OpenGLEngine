/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "FreeType.h"
#include <ft2build.h>
#include <freetype.h>
#include <ftstroke.h>
#include <ftsizes.h>
#include "edtaa3.h"
#include "DistanceField.h"
#include <conio.h>

namespace freetype
{
	// a single FT instance
	extern FT_Library ftLibrary;


	// aligns an integer value to 4: 36=>36, 37=>40, 38=>40 etc.
	template<class T> inline T alignTo4(T sz)
	{
		if (T rem = sz & 3)
			return sz + 4 - rem; // align up
		return sz; // already aligned
	}



	// emplaces a new glyph for the specified char into the glyphatlas (binary sort insert)
	Glyph* Font::GlyphAtlas::Emplace(wchar_t ch)
	{
		int imin = 0;
		if (int imax = mGlyphs.size())
		{
			while (imin < imax)
			{
				int imid = (imin + imax) >> 1;
				if (mGlyphs[imid].code < ch)
					imin = imid + 1;
				else
					imax = imid;
			}
			if (imin < imax && mGlyphs[imin].code == ch)
				return &mGlyphs[imin]; // the glyph already exists
		}
		return &*mGlyphs.emplace(mGlyphs.begin() + imin);
	}
	
	// retrieves a pointer to the specified codepoint in this glyphatlas
	// if a glyph can't be generated at all, the default glyph is returned
	// if a glyph hasn't been generated yet (but possible), NULL is returned
	const Glyph* Font::GlyphAtlas::Get(uint codepoint) const
	{
		ushort ch = ushort(codepoint);
		int imax = mGlyphs.size() - 1;
		// let's do a binary search
		int imin = 0;
		while (imin < imax)
		{
			int imid = (imin + imax) >> 1;
			if (mGlyphs[imid].code < ch)
				imin = imid + 1;
			else
				imax = imid;
		}
		if (imax == imin && mGlyphs[imin].code == ch)
			return &mGlyphs[imin]; // found it

		return NULL; // if a glyph doesn't exist yet, return NULL
	}



	// creates a new glyph
	// modifies: packX, packHeight, width, height
	void Font::GlyphAtlas::CreateGlyph(Glyph& g, Font::BufferImage& img, uint charcode, uint glyphIndex)
	{
		FT_Glyph strokeGlyph = nullptr;
		FT_Face face = FT_Face(font->face->ftFace);
		// load the glyph, but don't render it:
		FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_BITMAP|FT_LOAD_FORCE_AUTOHINT);

		// create a glyph from the provided metrics data
		const FT_Glyph_Metrics& metrics = face->glyph->metrics;
		if (!metrics.width || !metrics.height) // empty glyph?
		{
			if (!metrics.horiAdvance) // if no horizontal advance, us default glyph data
				g = mGlyphs[0]; // copy the default glyph data
			else
				g.advance = byte(metrics.horiAdvance >> 6); // set the horizontal advance (probably a space)
			g.code  = (ushort)charcode;   // change the charcode
			g.index = (ushort)glyphIndex; // and glyph index
			return; // done.
		}

		ushort glyphWidth, glyphHeight, advance;
		if (mStyle == FONT_PLAIN)
			glyphWidth = ushort(metrics.width >> 6), 
			glyphHeight = ushort(metrics.height >> 6),
			advance = ushort(metrics.horiAdvance >> 6);
		// shadow style just requires slight extra padding
		else if (mStyle == FONT_SHADOW)
			glyphWidth = ushort(metrics.width >> 6) + mPadding, 
			glyphHeight = ushort(metrics.height >> 6) + mPadding,
			advance = ushort(metrics.horiAdvance >> 6);
		
		// outline is just a stroke without the actual glyph
		else if (mStyle == FONT_STROKE || mStyle == FONT_OUTLINE)
		{
			FT_Get_Glyph(face->glyph, &strokeGlyph); // get the outline glyph
			FT_Glyph_Stroke(&strokeGlyph, FT_Stroker(ftStroker), true);

			FT_BBox cbox;
			FT_Glyph_Get_CBox(strokeGlyph, FT_GLYPH_BBOX_PIXELS, &cbox);
			glyphWidth = ushort(cbox.xMax - cbox.xMin);
			glyphHeight = ushort(cbox.yMax - cbox.yMin);
			advance = ushort((metrics.horiAdvance >> 6) + (mPadding));
		}


		if (mWidth < (mPackX + glyphWidth)) // glyph will no longer fit in this pack width?
		{
			// advance to next height 
			mPackY += mPackHeight + 16;             // +16 pixel padding between chars
			mPackX = 4, mPackHeight = glyphHeight; // reset pack X and pack Height
		}
		else if (glyphHeight > mPackHeight) // glyphHeight won't fit packing height?
			mPackHeight = glyphHeight;      // increase the packing height

		// this has to come after PackY has advanced
		if (MAX_TEXTURE_SIZE <= (mPackY + mPackHeight)) // if the texture no longer fits
		{
			mHeight = MAX_TEXTURE_SIZE;   // the height is fixed now
			*&g = mGlyphs[0];             // copy the default glyph
			g.code  = (ushort)charcode;   // its charcode
			g.index = (ushort)glyphIndex; // and the glyph index
			return; // done.
		}

		g.code     = (ushort)charcode;
		g.index    = (ushort)glyphIndex;
		g.width    = (byte)glyphWidth;
		g.height   = (byte)glyphHeight;
		g.advance  = (byte)advance;
		g.bearingX = char(metrics.horiBearingX >> 6);
		g.bearingY = char(metrics.horiBearingY >> 6);
		g.textureX = (byte)mPackX;
		g.textureY = mPackY; // set Y to current pack Y

		// advance to next x
		mPackX += glyphWidth + 16; // +16 pixel padding between chars
		mHeight = mPackY + mPackHeight;

		//// @brief Render glyph to texture
		switch (mStyle)
		{
			case FONT_PLAIN:
			{
				FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				FT_Bitmap& b = face->glyph->bitmap;
				img.InitImage(b.width, b.rows, 1);
				img.SetSubImage(0, 0, b.width, b.rows, b.buffer);
				break;
			}
			case FONT_SHADOW:
			{
				FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				FT_Bitmap& b = face->glyph->bitmap;

				if (!font->is_sdf) // regular (old) bitmap fonts:
				{
					img.InitImage(b.width + mPadding, b.rows + mPadding, 1);
					img.SetSubImage(0, mPadding, b.width, b.rows, b.buffer);
					// for shadows we just blit a copy of the original image to the background.
					img.MaskSubImage(mPadding, 0, b.width, b.rows, b.buffer);
				}
				else // SDF fonts only 1 channel right now
				{
					img.InitImage(b.width + mPadding, b.rows + mPadding, 1);
					img.SetSubImage(0, mPadding, b.width, b.rows, b.buffer);
				}
				break;
			}
			case FONT_OUTLINE:
			{
				// render stroke to bitmap
				FT_Glyph_To_Bitmap(&strokeGlyph, FT_RENDER_MODE_NORMAL, 0, true);

				// outline is always the main channel
				FT_Bitmap& b = FT_BitmapGlyph(strokeGlyph)->bitmap;
				img.InitImage(b.width, b.rows, 1);
				img.SetSubImage(0, 0, b.width, b.rows, b.buffer);
				break;
			}
			case FONT_STROKE:
			{
				int padLeft = ((int)g.width - (face->glyph->metrics.width >> 6)) / 2;	// (strokeW - normalW) / 2
				int padBottom = ((int)g.height - (face->glyph->metrics.height >> 6)) / 2; // (strokeH - normalH) / 2

				// render stroke to bitmap
				FT_Glyph_To_Bitmap(&strokeGlyph, FT_RENDER_MODE_NORMAL, 0, true);

				// turn the glyphslot into a BITMAP_GLYPH
				FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				FT_Bitmap* b = &face->glyph->bitmap;                 // main bitmap
				FT_Bitmap* s = &FT_BitmapGlyph(strokeGlyph)->bitmap; // stroke bitmap

				if (!font->is_sdf) // regular (old) bitmap fonts:
				{
					img.InitImage(s->width, s->rows, 2);
					img.SetSubImage(padLeft, padBottom, b->width, b->rows, b->buffer);
					img.MaskSubImage(0, 0, s->width, s->rows, s->buffer); // mask stroke
				}
				else // SDF fonts only 1 channel right now
				{
					img.InitImage(s->width, s->rows, 1);
					img.SetSubImage(padLeft, padBottom, b->width, b->rows, b->buffer);
				}
				break;
			}
		}
		if (strokeGlyph)
			FT_Done_Glyph(strokeGlyph);
	}




	// renders a glyph into the bufferimage
	void Font::GlyphAtlas::RenderGlyph(BufferImage& dst, const Glyph& g, const Font::BufferImage& img)
	{
		if (img.Data) // only if we have image data to copy
		{
			dst.CopySubImage(g.textureX, g.textureY, img);
		}
	}






	// destroys the entire atlas
	void Font::GlyphAtlas::Destroy()
	{
		mTexture.Destroy();
		mGlyphs.clear();
		if (ftStroker) FT_Stroker_Done(FT_Stroker(ftStroker)), ftStroker = 0;
		font = 0;
	}



	// creates the default subset of the atlas and initializes state variables
	void Font::GlyphAtlas::Create(Font* font, int fontHeight, FontStyle style, float outlineParam)
	{
		if (IsCreated())
			return; // don't do anything on already initialized atlas

		this->font = font;
		SelectFaceSize();
		font->is_sdf = false;

		FT_Face face = FT_Face(font->face->ftFace);
		uint maxGlyphWidth = ((face->bbox.xMax - face->bbox.xMin) * (fontHeight+1)) / face->units_per_EM;
		//uint maxGlyphHeight = ((face->bbox.yMax - face->bbox.yMin) * (fontHeight+1)) / face->units_per_EM;

		// setup stroker and padding.
		mStyle = style;
		if (style == FONT_SHADOW)
		{
			// shadow param is treated as an int and is always at least 1 pixel
			mPadding = std::max((int)outlineParam, 1);
		}
		else if (style == FONT_OUTLINE)
		{
			// outline param is the diameter of the outline, we have to *64/2, to get the radius
			int strokeRadius = int(outlineParam * 32.0f);
			mPadding = (int)outlineParam;
			FT_Stroker_New(ftLibrary, (FT_Stroker*)&ftStroker);
			FT_Stroker_Set(FT_Stroker(ftStroker), strokeRadius, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
		}
		else if (style == FONT_STROKE)
		{
			// add 0.5 pixels to the stroke (32), otherwise it looks bad
			// stroke param is the radius of the stroke, we have to *64
			int strokeRadius = int(outlineParam * 64.0f);
			mPadding = (int)outlineParam;
			FT_Stroker_New(ftLibrary, (FT_Stroker*)&ftStroker);
			FT_Stroker_Set(FT_Stroker(ftStroker), strokeRadius, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
		}

		// @note About padding and textures - OpenGL texture coordinates are cartesian (y0 at bottom)
		// get the maximum width in pixels
		mWidth = (ushort)maxGlyphWidth;
		if (style == FONT_SHADOW)
			mWidth += mPadding; // shadow has 1x padding
		else if(style == FONT_OUTLINE)
			mWidth += mPadding; // outline has 0.5*2 (=1x) padding
		if (style == FONT_STROKE) 
			mWidth += mPadding + mPadding; // stroke has 2x padding
		mWidth = mWidth <= 256 ? 256 : alignTo4(mWidth); // at least 128 px wide, or aligned to 4
		mHeight = 0;

		// initialize packing variables
		mPackX = 0, mPackY = 0;
		mPackHeight = 0;

		// only generate true ASCII chars [0..127]
		struct CCGI { uint ccode, glyphi; } cg[128];
		int ccode = 0, cgCount = 0;
		do
		{
			auto& c = cg[cgCount++];
			c.ccode = ccode = FT_Get_Next_Char(face, ccode, &c.glyphi);
		}
		while (ccode < 128);

		// create and render the glyphs
		mGlyphs.resize(cgCount);
		std::vector<BufferImage> renderedGlyphs(cgCount);
		for (int i = 0; i < cgCount; ++i)
		{
			auto& c = cg[i];
			CreateGlyph(mGlyphs[i], renderedGlyphs[i], c.ccode, c.glyphi);
		}
		if (mHeight != MAX_TEXTURE_SIZE) mHeight = alignTo4(mHeight); // align height to 4

		// number of channels in the bufferimage
		// PLAIN and OUTLINE only use 1 channel
		// SHADOW and STROKE use 2 channels
		// SDF fonts use 1 channel
		int channels = font->is_sdf ? 1 : (style == FONT_SHADOW || style == FONT_STROKE) ? 2 : 1;

		// create a temp buffer image for blitting the shadows and outlines
		// @note Remember - shadows/strokes mustn't fall under the main channel, so they're blitted.
		BufferImage image(mWidth, mHeight, channels);

		// now for each glyph, we render it into the texture
		for (int i = 0; i < cgCount; ++i)
			RenderGlyph(image, mGlyphs[i], renderedGlyphs[i]);

		if (font->is_sdf)
		{
			//BufferImage cdf(image.Width, image.Height, 1);
			//memcpy(cdf.Data, image.Data, image.Width*image.Height);
			//convert_to_sdf(cdf.Width, cdf.Height, (byte*)cdf.Data, 4.0f);

			BufferImage mdm;
			mdm.Data = (BufferImage::Pixel*)make_distance_map((byte*)image.Data, image.Width, image.Height);
			mdm.Width = image.Width;
			mdm.Height = image.Height;
			mdm.Channels = image.Channels;
			mTexture = Texture(mdm.Data, mdm.Width, mdm.Height, FMT_R);
		}
		else
		{
			mTexture = Texture(image.Data, image.Width, image.Height, channels == 1 ? FMT_R : FMT_RG);
		}
	}



	// generates new glyphs based on the input string
	// @note This function filters out glyphs that have been generated
	//       and only generates glyphs for missing glyphs
	//       Therefore you can call this anytime without much fear
	void Font::GlyphAtlas::GenerateGlyphs(const wchar_t* str, size_t len)
	{
		wchar_t* gen = (wchar_t*)alloca(sizeof(wchar_t) * len);
		int count = 0;
		for (; len; --len)
		{
			wchar_t ch = *str++;
			// check if this char is unique:
			if (wmemchr(gen, ch, count)) continue; // its not unique, skip
			if (Get(ch) != NULL) continue; // this glyph has been generated or is an invalid glyph
			
			gen[count++] = ch; // add to gen list
		}
		if (!count) return; // didn't exactly expect this

		FT_Face ftFace = FT_Face(font->face->ftFace);
		SelectFaceSize();

		// generate all the required glyphs
		std::vector<BufferImage> renderedGlyphs(count);
		for (int i = 0; i < count; i++)
		{
			wchar_t ch = gen[i];
			CreateGlyph(*Emplace(ch), renderedGlyphs[i], ch, FT_Get_Char_Index(ftFace, ch));
		}
		if (mHeight != MAX_TEXTURE_SIZE) mHeight = alignTo4(mHeight); // align height to 4

		// recreate the temp buffer image for blitting
		// the buffer image is initialized by the existing OpenGL texture
		BufferImage image(mTexture, mWidth, mHeight);

		// now for each glyph we render it into the new texture
		for (int i = 0; i < count; i++)
			// we have to use 'Get', because Emplace would have garbled the glyphs vector
			RenderGlyph(image, *Get(gen[i]), renderedGlyphs[i]);
		
		// replace the old texture
		mTexture.Create(image.Data, image.Width, image.Height);
	}


	void Font::GlyphAtlas::SelectFaceSize()
	{
		FT_Face(font->face->ftFace)->size = FT_Size(font->ftSize);
	}
	
} // namespace freetype