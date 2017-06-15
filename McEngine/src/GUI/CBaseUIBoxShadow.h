//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		box shadows
//
// $NoKeywords: $bshad
//===============================================================================//

// TODO: fix this

#ifndef CBASEUIBOXSHADOW_H
#define CBASEUIBOXSHADOW_H

#include "CBaseUIElement.h"

class Shader;
class GaussianBlur;
class RenderTarget;
class GaussianBlurKernel;

class CBaseUIBoxShadow : public CBaseUIElement
{
public:
	CBaseUIBoxShadow(Color color=COLOR(0, 0, 0, 0), float radius=0, float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIBoxShadow();

	ELEMENT_BODY(CBaseUIBoxShadow)

	virtual void draw(Graphics *g);
	void renderOffscreen(Graphics *g);

	void forceRedraw() {m_bNeedsRedraw = true;}

	CBaseUIBoxShadow *setColoredContent(bool coloredContent);
	CBaseUIBoxShadow *setColor(Color color);
	CBaseUIBoxShadow *setShadowColor(Color color);

	inline float getRadius() const {return m_fRadius;}

	virtual void onResized();

private:
	void render(Graphics *g);

	bool m_bNeedsRedraw;
	bool m_bColoredContent;

	float m_fRadius;

	Color m_shadowColor;
	Color m_color;

	GaussianBlur *m_blur;
};

class GaussianBlur
{
public:
	GaussianBlur(int x, int y, int width, int height, int kernelSize, float radius);
	~GaussianBlur();

	void draw(Graphics *g, int x, int y);
	void setColor(Color color);

	void enable();
	void disable(Graphics *g);

	void setSize(Vector2 size);

	inline const Vector2 getPos() const {return m_vPos;}
	inline const Vector2 getSize() const {return m_vSize;}

private:
	Vector2 m_vPos;
	Vector2 m_vSize;
	int m_iKernelSize;
	float m_fRadius;

	RenderTarget *m_rt;
	RenderTarget *m_rt2;
	GaussianBlurKernel *m_kernel;
	Shader *m_blurShader;
};

#endif

