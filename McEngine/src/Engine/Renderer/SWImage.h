//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		software rasterizer implementation of Image
//
// $NoKeywords: $swimg
//===============================================================================//

#ifndef SWIMAGE_H
#define SWIMAGE_H

#include "Image.h"

class SWImage : public Image
{
public:
	SWImage(UString filepath, bool mipmapped = false, bool keepInSystemMemory = false);
	SWImage(int width, int height, bool mipmapped = false, bool keepInSystemMemory = false);
	virtual ~SWImage() {destroy();}

	virtual void bind(unsigned int textureUnit = 0);
	virtual void unbind();

	virtual void setFilterMode(Graphics::FILTER_MODE filterMode);
	virtual void setWrapMode(Graphics::WRAP_MODE wrapMode);

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	unsigned int m_iTextureUnitBackup;
};

#endif
