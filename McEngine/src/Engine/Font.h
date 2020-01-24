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
class VertexArrayObject;

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
	McFont(UString filepath, int fontSize = 16, bool antialiasing = true, int fontDPI = 96);
	McFont(UString filepath, std::vector<wchar_t> characters, int fontSize = 16, bool antialiasing = true, int fontDPI = 96);
	virtual ~McFont() {destroy();}

	void drawString(Graphics *g, UString text);
	void drawTextureAtlas(Graphics *g);

	void setSize(int fontSize) {m_iFontSize = fontSize;}
	void setDPI(int dpi) {m_iFontDPI = dpi;}
	void setHeight(float height) {m_fHeight = height;}

	inline int getSize() const {return m_iFontSize;}
	inline int getDPI() const {return m_iFontDPI;}
	inline float getHeight() const {return m_fHeight;} // precomputed average height (fast)

	float getStringWidth(UString text) const;
	float getStringHeight(UString text) const;

	const GLYPH_METRICS &getGlyphMetrics(wchar_t ch) const;
	const bool hasGlyph(wchar_t ch) const;

	// ILLEGAL:
	inline TextureAtlas *getTextureAtlas() const {return m_textureAtlas;}

protected:
	void constructor(std::vector<wchar_t> characters, int fontSize, bool antialiasing, int fontDPI);

	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	bool addGlyph(wchar_t ch);

	void addAtlasGlyphToVao(Graphics *g, wchar_t ch, float &advanceX, VertexArrayObject *vao);

	int m_iFontSize;
	bool m_bAntialiasing;
	int m_iFontDPI;

	// glyphs
	TextureAtlas *m_textureAtlas;

	std::vector<wchar_t> m_vGlyphs;
	std::unordered_map<wchar_t, bool> m_vGlyphExistence;
	std::unordered_map<wchar_t, GLYPH_METRICS> m_vGlyphMetrics;

	float m_fHeight;

	GLYPH_METRICS m_errorGlyph;
};

#endif
