//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		container for dynamically merging multiple images into one
//
// $NoKeywords: $imgtxat
//===============================================================================//

#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include "Resource.h"

class Image;

class TextureAtlas : public Resource
{
public:
	TextureAtlas(int width = 512, int height = 512);
	virtual ~TextureAtlas() {destroy();}

	Vector2 put(int width, int height, Color *pixels) {return put(width, height, false, false, pixels);}
	Vector2 put(int width, int height, bool flipHorizontal, bool flipVertical, Color *pixels);

	void setPadding(int padding) {m_iPadding = padding;}

	inline int getWidth() const {return m_iWidth;}
	inline int getHeight() const {return m_iHeight;}
	inline Image *getAtlasImage() const {return m_atlasImage;}

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	int m_iPadding;

	int m_iWidth;
	int m_iHeight;

	Image *m_atlasImage;

	int m_iCurX;
	int m_iCurY;
	int m_iMaxHeight;
};

#endif
