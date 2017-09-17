//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		freetype font wrapper
//
// $NoKeywords: $fnt
//===============================================================================//

#include "Font.h"

#include "ResourceManager.h"
#include "VertexArrayObject.h"
#include "Engine.h"

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftbitmap.h>
#include <ftoutln.h>
#include <fttrigon.h>

void renderFTGlyphToTextureAtlas(FT_Library library, FT_Face face, wchar_t ch, Image *textureAtlas, bool antialiasing, std::unordered_map<wchar_t, McFont::GLYPH_METRICS> *glyphMetrics, int &curX, int &curY, int &maxX, int &maxY, bool calculateSizeOnly);

McFont::McFont(UString filepath, unsigned int fontSize, bool antialiasing) : Resource(filepath)
{
	m_textureAtlas = NULL;
	m_iFontSize = fontSize;
	m_bAntialiasing = antialiasing;

	m_fHeight = 1.0f;
	m_errorGlyph.advance_x = 10;
	m_errorGlyph.character = '?';
	m_errorGlyph.size = Vector2(1,1);
	m_errorGlyph.uv = Vector2(0,0);
	m_errorGlyph.top = 10;
	m_errorGlyph.width = 10;
}

void McFont::init()
{
	debugLog("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());

	// the default set of wchar_t glyphs which are loaded for every font
	std::vector<wchar_t> defaultGlyphs;
	for (int i=32; i<256; i++) // ASCII table of non-whitespace glyphs, including German Umlaute
	{
		defaultGlyphs.push_back((wchar_t)i);
	}


	// init freetype
	FT_Library library;
	if (FT_Init_FreeType(&library))
	{
		engine->showMessageError("Font Error", "FT_Init_FreeType() failed!");
		return;
	}

	// load font file
	FT_Face face;
	if (FT_New_Face(library, m_sFilePath.toUtf8(), 0, &face))
	{
		engine->showMessageError("Font Error", "Couldn't load font file!\nFT_New_Face() failed.");
		FT_Done_FreeType(library);
		return;
	}

	if (FT_Select_Charmap(face , ft_encoding_unicode))
	{
		engine->showMessageError("Font Error", "FT_Select_Charmap() failed!");
		return;
	}

	// set font height
	// "The character width and heights are specified in 1/64th of points"
	// HACKHACK: forcing 96 DPI
	unsigned int height = m_iFontSize;
	FT_Set_Char_Size(face, height*64, height*64, 96, 96);


	// get required atlas size
	// HACKHACK: the current logic forces a 512 pixel width (and dynamic height depending on the required size)
	// TODO: the atlas size calculation is bullshit
	const int defaultAtlasWidth = 512;
	const int defaultAtlasHeight = 256;
	engine->getResourceManager()->requestNextLoadUnmanaged();
	m_textureAtlas = engine->getResourceManager()->createImage(defaultAtlasWidth, defaultAtlasHeight);
	int curX = 1;
	int curY = 1;
	int maxX = 0;
	int maxY = 0;
	for (int i=0; i<defaultGlyphs.size(); i++)
	{
		renderFTGlyphToTextureAtlas(library, face, defaultGlyphs[i], m_textureAtlas, m_bAntialiasing, &m_vGlyphMetrics, curX, curY, maxX, maxY, true);
	}

	//int rawMinWidth = maxX;
	int rawMinHeight = curY + maxY;
	int atlasWidth = /*max((int)std::pow(2, std::ceil(std::log(rawMinWidth)/log(2))), defaultAtlasWidth)*/defaultAtlasWidth; // this must be defaultAtlasWidth as fallback (because we line wrap on overflowing widths during the size calculation)
	int atlasHeight = std::max((int)std::pow(2, std::ceil(std::log(rawMinHeight)/std::log(2))), 64);

	SAFE_DELETE(m_textureAtlas); // unmanaged, must delete manually
	engine->getResourceManager()->requestNextLoadUnmanaged();
	m_textureAtlas = engine->getResourceManager()->createImage(atlasWidth, atlasHeight);
	for (int x=0; x<m_textureAtlas->getWidth(); x++)
	{
		for (int y=0; y<m_textureAtlas->getHeight(); y++)
		{
			m_textureAtlas->setPixel(x, y, /*0xaa00aa00*/0x00000000);
		}
	}

	// now render all glyphs into the atlas
	curX = 1;
	curY = 1;
	maxX = 0;
	maxY = 0;
	for (int i=0; i<defaultGlyphs.size(); i++)
	{
		renderFTGlyphToTextureAtlas(library, face, defaultGlyphs[i], m_textureAtlas, m_bAntialiasing, &m_vGlyphMetrics, curX, curY, maxX, maxY, false);
	}


	// shutdown freetype
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	// build atlas texture
	m_textureAtlas->load();

	if (m_bAntialiasing)
		m_textureAtlas->setFilterMode(Graphics::FILTER_MODE::FILTER_MODE_LINEAR);
	else
		m_textureAtlas->setFilterMode(Graphics::FILTER_MODE::FILTER_MODE_NONE);


	// calculate generic height
	m_fHeight = 0.0f;
	for (int i=0; i<128; i++)
	{
		int curHeight = getGlyphMetrics((wchar_t)i).top;
		if (curHeight > m_fHeight)
			m_fHeight = curHeight;
	}

	m_bReady = true;
}

void McFont::initAsync()
{
	m_bAsyncReady = true;
}

void McFont::destroy()
{
	SAFE_DELETE(m_textureAtlas);

	m_vGlyphMetrics.clear();
	m_vGlyphMetrics.swap(m_vGlyphMetrics);
}

unsigned char* unpack_mono_bitmap(FT_Bitmap bitmap)
{
	unsigned char* result;
	int y, byte_index, num_bits_done, rowstart, bits, bit_index;
	unsigned char byte_value;

	result = new unsigned char[bitmap.rows * bitmap.width];

	for (y = 0; y < bitmap.rows; y++)
	{
		for (byte_index = 0; byte_index < bitmap.pitch; byte_index++)
		{
			byte_value = bitmap.buffer[y * bitmap.pitch + byte_index];
			num_bits_done = byte_index * 8;
			rowstart = y * bitmap.width + byte_index * 8;
			bits = 8;

			if ((bitmap.width - num_bits_done) < 8)
			{
				bits = bitmap.width - num_bits_done;
			}

			for (bit_index = 0; bit_index < bits; bit_index++)
			{
				int bit;
				bit = byte_value & (1 << (7 - bit_index));
				result[rowstart + bit_index] = bit;
			}
		}
	}

	return result;
}

void renderFTGlyphToTextureAtlas(FT_Library library, FT_Face face, wchar_t ch, Image *textureAtlas, bool antialiasing, std::unordered_map<wchar_t, McFont::GLYPH_METRICS> *glyphMetrics, int &curX, int &curY, int &maxX, int &maxY, bool calculateSizeOnly)
{
	// load current glyph
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), antialiasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO))
	{
		engine->showMessageError("Font Error", "FT_Load_Glyph() failed!");
		return;
	}

	// create glyph object from face glyph
    FT_Glyph glyph;
    if (FT_Get_Glyph(face->glyph, &glyph))
    {
    	engine->showMessageError("Font Error", "FT_Get_Glyph() failed!");
    	return;
    }

	// convert glyph to bitmap
	FT_Glyph_To_Bitmap(&glyph, antialiasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO, 0, 1);
    FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

    // get width & height of the glyph bitmap
    FT_Bitmap& bitmap = bitmapGlyph->bitmap;
	int width = bitmap.width;
	int height = bitmap.rows;

	if (!calculateSizeOnly)
	{
		// temp texture data memory
		unsigned char *expandedData = new unsigned char[4 * width * height];
		unsigned char *monoBitmapUnpacked = NULL;
		if (!antialiasing)
			monoBitmapUnpacked = unpack_mono_bitmap(bitmap);

		// expand bitmap
		if (antialiasing)
		{
			for (int j=0; j<height; j++)
			{
				for (int i=0; i<width; i++)
				{
					unsigned char alpha =  (i>=bitmap.width || j>=bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width*j];

					expandedData[(4*i + (height - j - 1) * width * 4)    ] = 0xff; // R
					expandedData[(4*i + (height - j - 1) * width * 4) + 1] = 0xff; // G
					expandedData[(4*i + (height - j - 1) * width * 4) + 2] = 0xff; // B
					expandedData[(4*i + (height - j - 1) * width * 4) + 3] = alpha;
				}
			}
		}
		else
		{
			for (int j=0; j<height; j++)
			{
				for (int i=0; i<width; i++)
				{
					unsigned char monoColor =  (i>=bitmap.width || j>=bitmap.rows) ? 0 : monoBitmapUnpacked[i + bitmap.width*j] > 0 ? 255 : 0;

					expandedData[(4*i + (height - j - 1) * width * 4)    ] = 0xff; // R
					expandedData[(4*i + (height - j - 1) * width * 4) + 1] = 0xff; // G
					expandedData[(4*i + (height - j - 1) * width * 4) + 2] = 0xff; // B
					expandedData[(4*i + (height - j - 1) * width * 4) + 3] = monoColor;
				}
			}
		}

		// TODO: refactor texture atlas logic
		// overflow to next line, also add 1 pixel border around everything
		if (curX + width + 1 > textureAtlas->getWidth()-1)
		{
			curX = 1;
			curY += maxY + 1;
			maxY = 0;
		}
		if (height > maxY)
			maxY = height;

		// add glyph to atlas
		if (curX+width < textureAtlas->getWidth() && curY+height < textureAtlas->getHeight())
		{
			for (int x=0; x<width; x++)
			{
				for (int y=0; y<height; y++)
				{
					char red = expandedData[(4*x + (height - y - 1) * width * 4)];
					char green = expandedData[(4*x + (height - y - 1) * width * 4) + 1];
					char blue = expandedData[(4*x + (height - y - 1) * width * 4) + 2];
					char alpha = expandedData[(4*x + (height - y - 1) * width * 4) + 3];

					textureAtlas->setPixel(curX + x, curY + y, COLOR(alpha, red, green, blue));
				}
			}
		}
		else
			engine->showMessageErrorFatal("Fatal Font Error", UString::format("%ix%i texture atlas is too small for glyph (%i, %i)!\n", textureAtlas->getWidth(), textureAtlas->getHeight(), curX+width, curY+height));


		(*glyphMetrics)[ch].uv = Vector2(curX, curY);
		(*glyphMetrics)[ch].size = Vector2(width, height);

		// go to next glyph spot in atlas
		curX += width + 1;

		// free temp expanded textures
		delete[] expandedData;
		if (!antialiasing)
			delete[] monoBitmapUnpacked;
	}
	else // calculate atlas size
	{
		// TODO: refactor texture atlas logic
		// overflow to next line, also add 1 pixel border around everything
		if (curX + width + 1 > textureAtlas->getWidth()-1)
		{
			curX = 1;
			curY += maxY + 1;
			maxY = 0;
			maxX = textureAtlas->getWidth();
		}
		if (height > maxY)
			maxY = height;

		Vector2 uv = Vector2(curX, curY);
		Vector2 size = Vector2(width, height);

		// save glyph metrics
		(*glyphMetrics)[ch] = (struct McFont::GLYPH_METRICS) {ch, uv, size, bitmapGlyph->left, bitmapGlyph->top, bitmap.width, bitmap.rows, (float)(face->glyph->advance.x >> 6)};

		// go to next glyph spot in atlas
		curX += width + 1;
	}

	// release
	FT_Done_Glyph(glyph);
}

void McFont::drawString(Graphics *g, UString text)
{
	if (!m_bReady) return;

	// texture atlas rendering
	m_textureAtlas->bind();
	m_worldMatrixBackup = g->getWorldMatrix();
	g->pushTransform();
	{
		for (int i=0; i<text.length(); i++)
		{
			drawAtlasGlyph(g, text[i]);
		}
	}
	g->popTransform();
}

void McFont::drawAtlasGlyph(Graphics *g, wchar_t ch)
{
	const GLYPH_METRICS &gm = getGlyphMetrics(ch);

	g->pushTransform();

		// apply font offsets and flip horizontally
		Matrix4 glyphMatrix;
		glyphMatrix.translate(gm.left, (gm.top - gm.rows), 0);
		glyphMatrix.scale(1, -1, 1);

		// this and the last few lines in this function are needed to keep outside transforms consistent
		// else we would e.g. scale every glyph by its center, instead of by the whole string
		Matrix4 finalMat = m_worldMatrixBackup * glyphMatrix;
		g->setWorldMatrix(finalMat);

		const float x = gm.uv.x/m_textureAtlas->getWidth();
		const float y = gm.uv.y/m_textureAtlas->getHeight();

		const float sx = gm.size.x/m_textureAtlas->getWidth();
		const float sy = gm.size.y/m_textureAtlas->getHeight();

		// draw it
		VertexArrayObject vao(Graphics::PRIMITIVE::PRIMITIVE_QUADS);

		vao.addTexcoord(x, y);
		vao.addVertex(0, gm.rows);

		vao.addTexcoord(x, y+sy);
		vao.addVertex(0, 0);

		vao.addTexcoord(x+sx, y+sy);
		vao.addVertex(gm.width,0);

		vao.addTexcoord(x+sx, y);
		vao.addVertex(gm.width,gm.rows);

		g->drawVAO(&vao);

	g->popTransform();

	// go to possible next glyph
	Matrix4 translateWorld;
	translateWorld.translate(gm.advance_x, 0, 0);
	m_worldMatrixBackup = m_worldMatrixBackup * translateWorld;
}

void McFont::drawTextureAtlas(Graphics *g)
{
	g->pushTransform();
		g->translate(m_textureAtlas->getWidth()/2 + 50, m_textureAtlas->getHeight()/2 + 50);
		g->drawImage(m_textureAtlas);
	g->popTransform();
}

void McFont::setSize(int fontSize)
{
	m_iFontSize = fontSize;
	reload();
}

float McFont::getStringWidth(UString text)
{
	if (!m_bReady) return 1.0f;

	float width = 0;
	for (int i=0; i<text.length(); i++)
	{
		width += getGlyphMetrics(text[i]).advance_x;
	}
	return width;
}

float McFont::getStringHeight(UString text)
{
	if (!m_bReady) return 1.0f;

	float height = 0;
	for (int i=0; i<text.length(); i++)
	{
		height += getGlyphMetrics(text[i]).top;
	}
	return height;
}

const McFont::GLYPH_METRICS &McFont::getGlyphMetrics(wchar_t ch)
{
	// G++/MinGW: HACKHACK: at() causes a crash if the element is not found? doesn't throw the expected exception
	if (m_vGlyphMetrics.find(ch) != m_vGlyphMetrics.end())
		return m_vGlyphMetrics.at(ch);
	else if (m_vGlyphMetrics.find(UNKNOWN_CHAR) != m_vGlyphMetrics.end())
		return m_vGlyphMetrics.at(UNKNOWN_CHAR);
	else
	{
		debugLog("Font Error: Missing default backup glyph (UNKNOWN_CHAR)!\n");
		return m_errorGlyph;
	}
}

const bool McFont::hasGlyph(wchar_t ch)
{
	if (m_vGlyphMetrics.find(ch) != m_vGlyphMetrics.end())
		return true;
	return false;
}
