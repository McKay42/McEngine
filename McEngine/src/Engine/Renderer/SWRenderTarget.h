//================== Copyright (c) 2016, PG, All rights reserved. ===================//
//
// Purpose:		software rasterizer implementation of RenderTarget / render to texture
//
// $NoKeywords: $swrt
//===================================================================================//

#ifndef SWRENDERTARGET_H
#define SWRENDERTARGET_H

#include "RenderTarget.h"

class SWRenderTarget : public RenderTarget
{
public:
	SWRenderTarget(int x, int y, int width, int height, Graphics::MULTISAMPLE_TYPE multiSampleType = Graphics::MULTISAMPLE_TYPE::MULTISAMPLE_0X);
	virtual ~SWRenderTarget() {destroy();}

	virtual void enable();
	virtual void disable();

	virtual void bind(unsigned int textureUnit = 0);
	virtual void unbind();

private:
	virtual void init();
	virtual void initAsync();
	virtual void destroy();
};

#endif
