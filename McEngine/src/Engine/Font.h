//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		freetype font wrapper
//
// $NoKeywords: $fnt
//===============================================================================//

#ifndef FONT_H
#define FONT_H

#include "Resource.h"

class Image;
class TextureAtlas;

class McFont : public Resource
{
public:
	static const wchar_t UNKNOWN_CHAR = 63; // ascii '?'

	struct GLYPH_METRICS
	{
		wchar_t character;

		unsigned int uvPixelsX;
		unsigned int uvPixelsY;
		unsigned int sizePixelsX;
		unsigned int sizePixelsY;

		int left;
		int top;
		int width;
		int rows;

		float advance_x;
	};

public:
	McFont(UString filepath, unsigned int fontSize = 16, bool antialiasing = true);
	McFont(UString filepath, std::vector<wchar_t> characters, unsigned int fontSize = 16, bool antialiasing = true);
	virtual ~McFont() {destroy();}

	void drawString(Graphics *g, UString text);
	void drawTextureAtlas(Graphics *g);

	void setSize(int fontSize);
	void setHeight(float height) {m_fHeight = height;}

	inline TextureAtlas *getTextureAtlas() const {return m_textureAtlas;}
	inline float getHeight() const {return m_fHeight;} // precomputed average height (fast)
	float getStringWidth(UString text);
	float getStringHeight(UString text);

	const GLYPH_METRICS &getGlyphMetrics(wchar_t ch);
	const bool hasGlyph(wchar_t ch);

protected:
	void constructor(std::vector<wchar_t> characters, unsigned int fontSize, bool antialiasing);

	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	bool addGlyph(wchar_t ch);

	void drawAtlasGlyph(Graphics *g, wchar_t ch);

	unsigned int m_iFontSize;
	bool m_bAntialiasing;

	// glyphs
	TextureAtlas *m_textureAtlas;

	std::vector<wchar_t> m_vGlyphs;
	std::unordered_map<wchar_t, bool> m_vGlyphExistence;
	std::unordered_map<wchar_t, GLYPH_METRICS> m_vGlyphMetrics;

	float m_fHeight;

	GLYPH_METRICS m_errorGlyph;

	// rendering
	Matrix4 m_worldMatrixBackup;
};

#endif
