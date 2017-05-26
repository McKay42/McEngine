//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		empty implementation of Image
//
// $NoKeywords: $nimg
//===============================================================================//

#ifndef NULLIMAGE_H
#define NULLIMAGE_H

#include "Image.h"

class NullImage : public Image
{
public:
	NullImage(UString filePath, bool mipmapped = false) : Image(filePath, mipmapped) {;}
	NullImage(int width, int height, bool mipmapped = false) : Image(width, height, mipmapped) {;}
	virtual ~NullImage() {destroy();}

	virtual void bind(unsigned int textureUnit = 0) {;}
	virtual void unbind() {;}

	virtual void setFilterMode(Graphics::FILTER_MODE filterMode) {;}
	virtual void setWrapMode(Graphics::WRAP_MODE wrapMode) {;}

private:
	virtual void init() {m_bReady = true;}
	virtual void initAsync() {m_bAsyncReady = true;}
	virtual void destroy() {;}
};

#endif
