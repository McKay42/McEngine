//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		freetype font wrapper
//
// $NoKeywords: $fnt
//===============================================================================//

#ifndef FONT_H
#define FONT_H

#include "Resource.h"

class McFont : public Resource
{
protected:
	const wchar_t UNKNOWN_CHAR = 63; // ascii '?'

public:
	McFont(UString filepath, unsigned int fontSize, bool antialiasing = true);
	virtual ~McFont() {destroy();}

	void drawString(Graphics *g, UString text);
	void drawTextureAtlas(Graphics *g);

	void setSize(int fontSize);

	inline Image *getTextureAtlas() {return m_textureAtlas;}
	float getStringWidth(UString text);
	inline float getHeight() const {return m_fHeight;}
	float getStringHeight(UString text);

	struct GLYPH_METRICS // this needs to be public because of the global renderFTGlyphToTextureAtlas() function
	{
		wchar_t character;
		Vector2 uv;
		Vector2 size;
		int left;
		int top;
		int width;
		int rows;
		float advance_x;
	};

	const GLYPH_METRICS &getGlyphMetrics(wchar_t ch);
	const bool hasGlyph(wchar_t ch);

protected:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	void drawAtlasGlyph(Graphics *g, wchar_t ch);

	unsigned int m_iFontSize;
	bool m_bAntialiasing;

	Image *m_textureAtlas;
	std::unordered_map<wchar_t, GLYPH_METRICS> m_vGlyphMetrics;
	GLYPH_METRICS m_errorGlyph;
	float m_fHeight;

	// rendering
	Matrix4 m_worldMatrixBackup;
};

#endif
