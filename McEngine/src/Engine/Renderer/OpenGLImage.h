//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		OpenGL implementation of Image
//
// $NoKeywords: $glimg
//===============================================================================//

#ifndef OPENGLIMAGE_H
#define OPENGLIMAGE_H

#include "Image.h"

#ifdef MCENGINE_FEATURE_OPENGL

class OpenGLImage : public Image
{
public:
	OpenGLImage(UString filepath, bool mipmapped = false, bool keepInSystemMemory = false);
	OpenGLImage(int width, int height, bool mipmapped = false, bool keepInSystemMemory = false);
	virtual ~OpenGLImage() {destroy();}

	virtual void bind(unsigned int textureUnit = 0);
	virtual void unbind();

	virtual void setFilterMode(Graphics::FILTER_MODE filterMode);
	virtual void setWrapMode(Graphics::WRAP_MODE wrapMode);

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();

	void handleGLErrors();

	unsigned int m_GLTexture;
	unsigned int m_iTextureUnitBackup;
};

#endif

#endif
