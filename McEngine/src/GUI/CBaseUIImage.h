//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple image class
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUIIMAGE_H
#define CBASEUIIMAGE_H

#include "CBaseUIElement.h"

class CBaseUIImage : public CBaseUIElement
{
public:
	CBaseUIImage(UString imageResourceName="", float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIImage(){;}

	virtual void draw(Graphics *g);

	void setImage(Image *img);

	CBaseUIImage *setDrawFrame(bool drawFrame){m_bDrawFrame = drawFrame; return this;}
	CBaseUIImage *setDrawBackground(bool drawBackground){m_bDrawBackground = drawBackground; return this;}

	CBaseUIImage *setFrameColor(Color frameColor){m_frameColor = frameColor; return this;}
	CBaseUIImage *setColor(Color color){m_color = color; return this;}
	CBaseUIImage *setAlpha(float alpha){m_color &= 0x00ffffff; m_color |= ((int)(255.0f * alpha)) << 24; return this;}
	CBaseUIImage *setBackgroundColor(Color backgroundColor){m_backgroundColor = backgroundColor; return this;}

	CBaseUIImage *setRotationDeg(float rotation){m_fRot = rotation; return this;}
	CBaseUIImage *setScale(float xScale, float yScale){m_vScale.x = xScale;m_vScale.y = yScale; return this;}
	CBaseUIImage *setScale(Vector2 scale){m_vScale.x = scale.x; m_vScale.y = scale.y; return this;}
	CBaseUIImage *setScaleToFit(bool scaleToFit) {m_bScaleToFit = scaleToFit; return this;}

	inline float getRotationDeg() const {return m_fRot;}
	inline Vector2 getScale() const {return m_vScale;}
	inline Image *getImage() const {return m_image;}

	// Overrides
	virtual CBaseUIImage *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIImage *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIImage *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIImage *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIImage *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIImage *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIImage *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIImage *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIImage *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIImage *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIImage *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIImage *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIImage *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIImage *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIImage *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIImage *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIImage *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIImage *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIImage *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIImage *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIImage *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIImage *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIImage *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIImage *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIImage *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIImage *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIImage *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIImage *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIImage *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIImage *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIImage *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIImage *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIImage *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIImage *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIImage *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIImage *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIImage *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIImage *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIImage *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIImage *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIImage *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIImage *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIImage *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIImage *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIImage *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIImage *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIImage *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIImage *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIImage *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIImage *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


private:
	Image *m_image;

	Color m_frameColor;
	Color m_backgroundColor;
	Color m_color;

	bool m_bDrawFrame;
	bool m_bDrawBackground;
	bool m_bScaleToFit;

	float m_fRot;
	Vector2 m_vScale;
};

#endif
