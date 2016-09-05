//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		image wrapper
//
// $NoKeywords: $img
//===============================================================================//

#ifndef IMAGE_H
#define IMAGE_H

#include "Resource.h"

class Image : public Resource
{
public:
	static void saveToImage(unsigned char *data, unsigned int width, unsigned int height, UString filepath);

	enum WRAP_MODE
	{
		WRAP_MODE_CLAMP,
		WRAP_MODE_REPEAT
	};

	enum FILTER_MODE
	{
		FILTER_MODE_NONE,
		FILTER_MODE_LINEAR,
		FILTER_MODE_MIPMAP
	};

	Image(ResourceManager *loader, UString filepath, bool mipmapped = false);
	Image(int width, int height, bool clampToEdge = true); // TODO: HACKHACK: backwards compatibility for CGProject; remove sometime in the future
	virtual ~Image();

	virtual void bind(unsigned int textureUnit = 0) = 0;
	virtual void unbind() = 0;

	virtual void setFilterMode(FILTER_MODE filterMode) = 0;
	virtual void setWrapMode(WRAP_MODE wrapMode) = 0;

	Color getPixel(int x, int y);
	void setPixel(int x, int y, Color color);

	inline int getWidth() const {return m_iWidth;}
	inline int getHeight() const {return m_iHeight;}
	inline Vector2 getSize() {return Vector2(m_iWidth, m_iHeight);}

	inline std::vector<unsigned char> *getRawImage() {return &m_rawImage;}

	// HACKHACK: TEMP: backwards compatibility for MetroidModelViewer
	void writeToFile(UString folder);

protected:
	virtual void init() = 0;
	virtual void initAsync() = 0;
	virtual void destroy() = 0;

	unsigned int m_iWidth;
	unsigned int m_iHeight;

	bool m_bMipmapped;
	bool m_bCreatedImage;
	bool m_bClampToEdge;
	std::vector<unsigned char> m_rawImage;
};

#endif
