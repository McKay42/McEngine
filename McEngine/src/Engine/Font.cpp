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
#include "ConVar.h"

#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftbitmap.h>
#include <ftoutln.h>
#include <fttrigon.h>

ConVar r_debug_drawstring_unbind("r_debug_drawstring_unbind", false);

const wchar_t McFont::UNKNOWN_CHAR;

void renderFTGlyphToTextureAtlas(FT_Library library, FT_Face face, wchar_t ch, TextureAtlas *textureAtlas, bool antialiasing, std::unordered_map<wchar_t, McFont::GLYPH_METRICS> *glyphMetrics);
unsigned char *unpackMonoBitmap(FT_Bitmap bitmap);

McFont::McFont(UString filepath, unsigned int fontSize, bool antialiasing) : Resource(filepath)
{
	// the default set of wchar_t glyphs (ASCII table of non-whitespace glyphs, including cyrillics)
	std::vector<wchar_t> characters;
	for (int i=32; i<255; i++)
	{
		characters.push_back((wchar_t)i);
	}

	constructor(characters, fontSize, antialiasing);
}

McFont::McFont(UString filepath, std::vector<wchar_t> characters, unsigned int fontSize, bool antialiasing) : Resource(filepath)
{
	constructor(characters, fontSize, antialiasing);
}

void McFont::constructor(std::vector<wchar_t> characters, unsigned int fontSize, bool antialiasing)
{
	for (int i=0; i<characters.size(); i++)
	{
		addGlyph(characters[i]);
	}

	m_iFontSize = fontSize;
	m_bAntialiasing = antialiasing;

	m_textureAtlas = NULL;

	m_fHeight = 1.0f;

	m_errorGlyph.character = '?';
	m_errorGlyph.advance_x = 10;
	m_errorGlyph.sizePixelsX = 1;
	m_errorGlyph.sizePixelsY = 1;
	m_errorGlyph.uvPixelsX = 0;
	m_errorGlyph.uvPixelsY = 0;
	m_errorGlyph.top = 10;
	m_errorGlyph.width = 10;
}

void McFont::init()
{
	debugLog("Resource Manager: Loading %s\n", m_sFilePath.toUtf8());

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

	// create texture atlas
	engine->getResourceManager()->requestNextLoadUnmanaged();
	m_textureAtlas = engine->getResourceManager()->createTextureAtlas(512, 512); // HACKHACK: hardcoded max atlas size!

	// now render all glyphs into the atlas
	for (int i=0; i<m_vGlyphs.size(); i++)
	{
		renderFTGlyphToTextureAtlas(library, face, m_vGlyphs[i], m_textureAtlas, m_bAntialiasing, &m_vGlyphMetrics);
	}

	// shutdown freetype
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	// build atlas texture
	engine->getResourceManager()->loadResource(m_textureAtlas);

	if (m_bAntialiasing)
		m_textureAtlas->getAtlasImage()->setFilterMode(Graphics::FILTER_MODE::FILTER_MODE_LINEAR);
	else
		m_textureAtlas->getAtlasImage()->setFilterMode(Graphics::FILTER_MODE::FILTER_MODE_NONE);

	// calculate average ASCII glyph height
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

	m_vGlyphMetrics = std::unordered_map<wchar_t, GLYPH_METRICS>();
	m_fHeight = 1.0f;
}

bool McFont::addGlyph(wchar_t ch)
{
	if (m_vGlyphExistence.find(ch) != m_vGlyphExistence.end()) return false;
	if (ch < 32) return false;

	m_vGlyphs.push_back(ch);
	m_vGlyphExistence[ch] = true;

	return true;
}

void McFont::drawString(Graphics *g, UString text)
{
	if (!m_bReady) return;

	// texture atlas rendering
	m_textureAtlas->getAtlasImage()->bind();
	m_worldMatrixBackup = g->getWorldMatrix();
	g->pushTransform();
	{
		for (int i=0; i<text.length(); i++)
		{
			drawAtlasGlyph(g, text[i]);
		}
	}
	g->popTransform();

	// NOTE: there is no unbind() of the atlas texture on purpose, for performance reasons
	if (r_debug_drawstring_unbind.getBool())
		m_textureAtlas->getAtlasImage()->unbind();
}

void McFont::drawAtlasGlyph(Graphics *g, wchar_t ch)
{
	// TODO: dynamic glyph loading, finish this
	/*
	if (m_vGlyphExistence.find(ch) == m_vGlyphExistence.end())
	{
		if (addGlyph(ch))
			reload();
	}
	*/

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

		const float x = (float)gm.uvPixelsX / (float)m_textureAtlas->getAtlasImage()->getWidth();
		const float y = (float)gm.uvPixelsY / (float)m_textureAtlas->getAtlasImage()->getHeight();

		const float sx = (float)gm.sizePixelsX / (float)m_textureAtlas->getAtlasImage()->getWidth();
		const float sy = (float)gm.sizePixelsY / (float)m_textureAtlas->getAtlasImage()->getHeight();

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
	{
		g->translate(m_textureAtlas->getWidth()/2 + 50, m_textureAtlas->getHeight()/2 + 50);
		g->drawImage(m_textureAtlas->getAtlasImage());
	}
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
	return (m_vGlyphMetrics.find(ch) != m_vGlyphMetrics.end());
}



// helper functions

void renderFTGlyphToTextureAtlas(FT_Library library, FT_Face face, wchar_t ch, TextureAtlas *textureAtlas, bool antialiasing, std::unordered_map<wchar_t, McFont::GLYPH_METRICS> *glyphMetrics)
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

	// build texture
	Vector2 atlasPos;
	if (width > 0 && height > 0)
	{
		// temp texture data memory
		unsigned char *expandedData = new unsigned char[4 * width * height];
		unsigned char *monoBitmapUnpacked = NULL;

		if (!antialiasing)
			monoBitmapUnpacked = unpackMonoBitmap(bitmap);

		// expand bitmap
		for (int j=0; j<height; j++)
		{
			for (int i=0; i<width; i++)
			{
				unsigned char alpha = 0;

				if (i < bitmap.width && j < bitmap.rows)
				{
					if (antialiasing)
						alpha = bitmap.buffer[i + bitmap.width*j];
					else
						alpha = monoBitmapUnpacked[i + bitmap.width*j] > 0 ? 255 : 0;
				}

				expandedData[(4*i + (height - j - 1) * width * 4)    ] = 0xff;	// R
				expandedData[(4*i + (height - j - 1) * width * 4) + 1] = 0xff;	// G
				expandedData[(4*i + (height - j - 1) * width * 4) + 2] = 0xff;	// B
				expandedData[(4*i + (height - j - 1) * width * 4) + 3] = alpha;	// A
			}
		}

		// add glyph to atlas
		atlasPos = textureAtlas->put(width, height, false, true, (Color*)expandedData);

		// free temp expanded textures
		delete[] expandedData;
		if (!antialiasing)
			delete[] monoBitmapUnpacked;
	}

	// save glyph metrics
	(*glyphMetrics)[ch].character = ch;

	(*glyphMetrics)[ch].uvPixelsX = (unsigned int)atlasPos.x;
	(*glyphMetrics)[ch].uvPixelsY = (unsigned int)atlasPos.y;
	(*glyphMetrics)[ch].sizePixelsX = (unsigned int)width;
	(*glyphMetrics)[ch].sizePixelsY = (unsigned int)height;

	(*glyphMetrics)[ch].left = bitmapGlyph->left;
	(*glyphMetrics)[ch].top = bitmapGlyph->top;
	(*glyphMetrics)[ch].width = bitmap.width;
	(*glyphMetrics)[ch].rows = bitmap.rows;

	(*glyphMetrics)[ch].advance_x = (float)(face->glyph->advance.x >> 6);

	// release
	FT_Done_Glyph(glyph);
}

unsigned char *unpackMonoBitmap(FT_Bitmap bitmap)
{
	unsigned char *result;
	int y, byte_index, num_bits_done, rowstart, bits, bit_index;
	unsigned char byte_value;

	result = new unsigned char[bitmap.rows * bitmap.width];

	for (y=0; y<bitmap.rows; y++)
	{
		for (byte_index=0; byte_index<bitmap.pitch; byte_index++)
		{
			byte_value = bitmap.buffer[y * bitmap.pitch + byte_index];
			num_bits_done = byte_index * 8;
			rowstart = y * bitmap.width + byte_index * 8;
			bits = 8;

			if ((bitmap.width - num_bits_done) < 8)
				bits = bitmap.width - num_bits_done;

			for (bit_index=0; bit_index<bits; bit_index++)
			{
				int bit;
				bit = byte_value & (1 << (7 - bit_index));
				result[rowstart + bit_index] = bit;
			}
		}
	}

	return result;
}
