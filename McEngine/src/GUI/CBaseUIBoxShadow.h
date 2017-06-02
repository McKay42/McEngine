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

	virtual void draw(Graphics *g);
	void renderOffscreen(Graphics *g);

	void forceRedraw() {m_bNeedsRedraw = true;}

	CBaseUIBoxShadow *setColoredContent(bool coloredContent);
	CBaseUIBoxShadow *setColor(Color color);
	CBaseUIBoxShadow *setShadowColor(Color color);

	// Overrides
	virtual CBaseUIBoxShadow *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIBoxShadow *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIBoxShadow *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIBoxShadow *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIBoxShadow *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIBoxShadow *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIBoxShadow *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIBoxShadow *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIBoxShadow *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIBoxShadow *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIBoxShadow *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIBoxShadow *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIBoxShadow *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIBoxShadow *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIBoxShadow *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIBoxShadow *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIBoxShadow *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIBoxShadow *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIBoxShadow *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIBoxShadow *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIBoxShadow *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIBoxShadow *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIBoxShadow *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIBoxShadow *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIBoxShadow *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIBoxShadow *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIBoxShadow *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIBoxShadow *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIBoxShadow *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIBoxShadow *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIBoxShadow *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIBoxShadow *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIBoxShadow *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIBoxShadow *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIBoxShadow *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIBoxShadow *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIBoxShadow *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIBoxShadow *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIBoxShadow *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIBoxShadow *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIBoxShadow *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIBoxShadow *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIBoxShadow *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIBoxShadow *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIBoxShadow *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIBoxShadow *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIBoxShadow *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIBoxShadow *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIBoxShadow *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIBoxShadow *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

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

