/**
 * Copyright (c) 2013 - Jorma Rebane
 */
#include "FreeType.h"
#include <ft2build.h>
#include <freetype.h>
#include <ftsizes.h>
#include <ftmodapi.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Timer.h"

namespace freetype
{
	// a single FT instance
	extern FT_Library ftLibrary;

	static HANDLE FontHeap = HeapCreate(0, 64 * 1024, 0);

	void* pooled_Alloc(FT_Memory memory, long size)
	{
		return HeapAlloc(FontHeap, 0, size);
	}
	void pooled_Free(FT_Memory memory, void* ptr)
	{
		if (ptr)
			HeapFree(FontHeap, 0, ptr);
	}
	void* pooled_Realloc(FT_Memory memory, long cur_size, long new_size, void* ptr)
	{
		if (!ptr)
			return HeapAlloc(FontHeap, 0, new_size);
		if (!new_size)
		{
			HeapFree(FontHeap, 0, ptr);
			return NULL;
		}
		return HeapReAlloc(FontHeap, 0, ptr, new_size);
	}



	static FT_MemoryRec_ ftMemoryPool = 
	{
		0, // user data (wish we had any)
		pooled_Alloc,
		pooled_Free,
		pooled_Realloc 
	};




	/**
	 * Loads a specific TrueType .TTF font file and generates a FreeType face out of it
	 * @param fontFile Name of the font file to load
	 * @return TRUE on success
	 */
	bool FontFace::CreateFromFile(const char* fontFile)
	{
		if (IsCreated())
			Destroy(); // destroy self

		SECURITY_ATTRIBUTES secu = { sizeof(secu), NULL, TRUE };
		HANDLE hFile = CreateFileA(fontFile, FILE_GENERIC_READ, 
								   FILE_SHARE_READ|FILE_SHARE_WRITE, &secu, 
								   OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0);
		if (hFile == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Font::LoadFile: file '%s' not found.\n", fontFile);
			return false;
		}

		int alignedSize = GetFileSize(hFile, NULL);
		if (int rem = alignedSize % 4096)
			alignedSize = (alignedSize - rem) + 4096;

		byte* buffer = (byte*)malloc(alignedSize);
		DWORD bytesRead;
		ReadFile(hFile, buffer, alignedSize, &bytesRead, NULL);
		CloseHandle(hFile);

		// @note data is freed by Destroy()
		return Create(buffer, bytesRead);
	}

	/**
	 * Loads a TrueType font data and generates a FreeType face out of it
	 * @param fontData Raw font file data
	 * @param dataLength Raw font file data length (in bytes)
	 * @return TRUE on success
	 */
	bool FontFace::Create(const byte* fontData, size_t dataLength)
	{
		if (!ftLibrary)
		{
			if (FT_New_Library(&ftMemoryPool, &ftLibrary))
			{
				fprintf(stderr, "FontFace::Create() failed: FT_Init_Freetype error.\n");
				return false;
			}
			FT_Add_Default_Modules(ftLibrary);
		}


		FT_Face face;
		if (FT_New_Memory_Face(ftLibrary, fontData, dataLength, 0, &face))
		{
			fprintf(stderr, "FontFace::Create() failed. Only TTF fonts are supported.\n");
			return false;
		}
		FT_Done_Size(face->size); // we need to delete this size object, otherwise we'll get a leak later
		face->size = 0;

		ftFace = face;
		fontfamily = face->family_name;
		fontfamily.push_back('-');
		fontfamily.append(face->style_name);
		return true;
	}

	/**
	 * Destroys a created FontFace
	 * @param freeData Set TRUE if you want data to be freed upon face destruction
	 * You would set this false if you manage the data outside of the FontFace class.
	 */
	void FontFace::Destroy(bool freeData)
	{
		fontfamily.clear(); // clear the family string
		if (ftFace)
			FT_Done_Face(FT_Face(ftFace)), ftFace = 0; // free the face
		if (data)
		{
			if (freeData) free(data); // free data bytes
			data = 0;
		}
	}

	/**
	 * Creates a Font based on the provided font height
	 * @param outlineParam Depends on the specified FontStyle
	 *				FONT_PLAIN - param is ignored
	 *				FONT_SHADOW - param is the shadow offset in pixels (int)
	 *				FONT_OUTLINE - param is the outline width in sub-pixels (float)
	 *				FONT_STROKE - param is the stroke width in sub-pixels (float)
	 */
	Font* FontFace::NewFont(unsigned fontHeight, FontStyle style, float outlineOffset, int dpi)
	{
		if (this->ftFace == NULL)
			return NULL; // can't create a new font if no face loaded

		Font* font = new Font();
		if (font->Create(this, fontHeight, style, outlineOffset, dpi))
			return font; // success
		delete font; // failed; delete font and return nothing
		return NULL;
	}

} // namespace freetype