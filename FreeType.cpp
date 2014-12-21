/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "FreeType.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_SIZES_H
#include "GL\glew.h"
#include "GL\glut.h"
#include "Timer.h"

namespace freetype
{
	// a single FT instance
	FT_Library ftLibrary = NULL;


	/** @return Distance in pixels to a newline */
	int Font::LineHeight() const
	{
		return (int)FT_Face(face->ftFace)->size->metrics.height >> 6;
	}

	
	
	/**
	 * Creates a Font based on the provided FontFace
	 */
	bool Font::Create(FontFace* face, unsigned fontHeight, FontStyle style, float outlineParam, int dpi)
	{
		Timer t(tstart);
		if (atlas.IsCreated())
			atlas.Destroy(); // unload previous atlas
		
		this->face = face;
		FT_Face ftFace = FT_Face(face->ftFace);

		FT_New_Size(ftFace, (FT_Size*)&ftSize);
		FT_Activate_Size(FT_Size(ftSize));
		FT_Set_Char_Size(ftFace, 0, fontHeight * 64, 0, dpi);

		this->height = (ushort)fontHeight;
		this->dpi = (ushort)dpi;

		// this is the actual heavyweight initializer:
		atlas.Create(this, fontHeight, style, outlineParam);
		printf("Font.Create(\"%s\", %dpx): %.1fms\n", face->fontfamily.c_str(), fontHeight, t.StopElapsed() * 1000);

		return true;
	
	}

	void Font::Destroy()
	{
		atlas.Destroy();
		if (ftSize) {
			FT_Face ftFace = FT_Face(face->ftFace);
			if (ftFace->size == ftSize) ftFace->size = 0; // unbind size from the face (otherwise it might be freed)
			FT_Done_Size(FT_Size(ftSize)), ftSize = 0; // delete the size object
		}
		face = 0;
	}






	Text* Font::CreateTextF(const wchar_t* fmt, ...)
	{
		va_list ap; va_start(ap, fmt);
		return CreateTextV(fmt, ap);
	}
	Text* Font::CreateTextV(const wchar_t* fmt, va_list ap)
	{
		// try a reasonable 8KB buffer
		wchar_t* buffer = (wchar_t*)alloca(sizeof(wchar_t) * 4096);
		int count = vswprintf(buffer, 4096, fmt, ap);
		if (count == -1) return NULL; // failed
		return CreateText(buffer, count);
	}

	Text* Font::CreateText(const wchar_t* str)
	{
		return CreateText(str, wcslen(str));
	}

	Text* Font::CreateText(const wchar_t* str, size_t len)
	{
		if (!atlas.IsCreated()) return NULL;
		
		Text* out = new Text();
		// we generate 6 vertices instead of using an index buffer, because:
		// 1) It uses less memory for text 2) There is no performance difference (measured with huge blocks of text)
		VertexText* vertices = (VertexText*)alloca(sizeof(VertexText) * 6 * len);
		size_t vertexCount = GenBlock(vertices, out->size, str, len);
		out->vb.Create<VertexText>();
		out->vb.BufferVertices(vertices, vertexCount);
		out->font = this;
		return out;
	}


	size_t Font::GenBlock(VertexText* outVerts, Vector2& outSize, const wchar_t* str, size_t len)
	{
		if (!atlas.IsCreated()) return NULL;
		
		size_t vertexCount = 0; // we don't pre-determine indexes, because not all chars generate vertices

		float x = 0.0f, y = (float)-(int)FontHeight();
		float lineHeight = (float)LineHeight(); // Y advance to next line
		Vector2 bounds; // bounds of the text

		// 0\``3
		// | \ |
		// 1__\2
		for (VertexText* quad = outVerts; len; --len)
		{
			wchar_t ch = *str++;
			if (ch == L'\n') // newline
			{
				x = 0.0f, y -= lineHeight;
				continue;
			}
			if (ch == L'\t') // horizontal tab (4 spaces)
			{
				x += (int)atlas.Get(L' ')->advance * 4;
				continue;
			}

			const Glyph* glyph = atlas.Get(ch);
			if (glyph == NULL) // a glyph needs to be generated
			{
				atlas.GenerateGlyphs(str-1, len); // generate any missing glyphs from this point onward
				glyph = atlas.Get(ch); // this has to succeed now
			}
			const Glyph& g = *glyph;
			if (g.width && g.height) // only generate face for glyphs with size
			{
				// vertex positions
				float x0 = x + (float)(int)g.bearingX;
				float x1 = x0 + (float)(int)g.width;
				float y0 = y + (float)(int)g.bearingY;
				float y1 = y0 - (float)(int)g.height;

				// update bounds
				if (x1 > bounds.x) bounds.x = x1;
				if (y1 < bounds.y) bounds.y = y1;

				// texture coordinates (in pixel values)
				float tx0 = (float)(int)g.textureX;
				float tx1 = (float)((int)g.textureX + (int)g.width);
				float ty0 = (float)((int)g.textureY + (int)g.height); // texture y0
				float ty1 = (float)(int)g.textureY;

				// *quad is indexed for cpu speed
				quad[0].x = x0, quad[0].y = y0, quad[0].u = tx0, quad[0].v = ty0; // top left
				quad[1].x = x0, quad[1].y = y1, quad[1].u = tx0, quad[1].v = ty1; // bottom left
				quad[2].x = x1, quad[2].y = y1, quad[2].u = tx1, quad[2].v = ty1; // bottom right

				quad[3] = quad[0]; // reuse top left
				quad[4] = quad[2]; // reuse bottom right
				quad[5].x = x1, quad[5].y = y0, quad[5].u = tx1, quad[5].v = ty0; // top right

				vertexCount += 6;	// update actual vertex count
				quad += 6;			// move to the next quad
			}
			x += (int)g.advance;
		}

		bounds.y = -bounds.y; // convert the y value to a size value
		outSize = bounds;
		return vertexCount;
	}




	/** @return Type of this text object (Static/Dynamic) */
	BufferType Text::Type() const
	{
		return (BufferType)vb.Type;
	}
	/** @return Number of glyphs in this text */
	size_t Text::Length() const
	{
		return vb.VertexCount / 6;
	}
	/** @return Specified glyphs XY offset */
	Vector2 Text::GlyphXY(size_t i) const
	{
		VertexText* v = (VertexText*)((VertexBuffer*)&vb)->MapVBO(MAP_RO);
		float x = v[i * 6].x;
		float y = v[i * 6].y;
		((VertexBuffer*)&vb)->UnmapVBO();
		return Vector2(x, y);
	}

	/**
	 * Draws this text segment on the screen
	 * @param mvp The model-view-projection matrix to translate the text position+scale+rotation
	 * @param color Color of the text
	 * @param outline Color of the stroke or shadow
	 */
	void Text::Draw(const Matrix4& mvp, const Vector4& color, const Vector4& outline)
	{
		IShaderProgram* shader = IShaderProgram::CurrentShader();
		shader->BindMatrix(mvp);
		shader->BindTexture(&font->atlas.mTexture);
		shader->BindDiffuseColor(color);
		shader->BindOutlineColor(outline);
		vb.Draw();
	}

	/**
	 * Draws a sub-segment of the text on the screen
	 * @param start Start index of the sub-segment of text to render
	 * @param count Number of characters to render
	 * @param mvp The model-view-projection matrix to translate the text position+scale+rotation
	 * @param color Color of the text
	 * @param outline Color of the stroke or shadow
	 */
	void Text::Draw(uint start, uint count, const Matrix4& mvp, const Vector4& color, const Vector4& outline)
	{
		IShaderProgram* shader = IShaderProgram::CurrentShader();
		shader->BindMatrix(mvp);
		shader->BindTexture(&font->atlas.mTexture);
		shader->BindDiffuseColor(color);
		shader->BindOutlineColor(outline);
		vb.Draw(start, count);
	}



	/**
	 * @brief Creates or Re-Creates this text object based on input string
	 * @param str String to create text from
	 * @param len Length of the string
	 */
	void Text::Create(const wchar_t* str, size_t len)
	{
		Vector2 sz;
		VertexText* vertices = (VertexText*)alloca(sizeof(VertexText) * 6 * len);
		size_t vertexCount = font->GenBlock(vertices, sz, str, len);
		vb.UpdateVertices(vertices, vertexCount);
	}

	/**
	 * @brief Appends new text to the end of this text
	 * @param str String to create text from
	 * @param len Length of the string
	 */
	void Text::Append(const wchar_t* str, size_t len)
	{
		Vector2 sz;
		VertexText* vertices = (VertexText*)alloca(sizeof(VertexText) * 6 * len);
		size_t vertexCount = font->GenBlock(vertices, sz, str, len);
		vb.AppendVertices(vertices, vertexCount);
	}

	/**
	 * @brief Inserts new text inside of this text
	 * @param str String to create text from
	 * @param len Length of the string
	 */
	void Text::Insert(int index, const wchar_t* str, size_t len)
	{
		Vector2 sz;
		VertexText* vertices = (VertexText*)alloca(sizeof(VertexText) * 6 * len);
		size_t vertexCount = font->GenBlock(vertices, sz, str, len);
		vb.InsertVertices(vertices, vertexCount);
	}

} // namespace freetype