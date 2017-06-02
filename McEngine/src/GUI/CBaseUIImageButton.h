//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a simple image button
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUIIMAGEBUTTON_H
#define CBASEUIIMAGEBUTTON_H

#include "CBaseUIButton.h"

class CBaseUIImageButton : public CBaseUIButton
{
public:
	CBaseUIImageButton(UString imageResourceName="", float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIImageButton() {;}

	virtual void draw(Graphics *g);

	virtual void onResized();

	CBaseUIImageButton *setImageResourceName(UString imageResourceName);
	CBaseUIImageButton *setRotationDeg(float deg) {m_fRot = deg; return this;}
	CBaseUIImageButton *setScale(float xScale, float yScale) {m_vScale.x = xScale; m_vScale.y = yScale; return this;}
	CBaseUIImageButton *setScaleToFit(bool scaleToFit) {m_bScaleToFit = scaleToFit; return this;}
	CBaseUIImageButton *setKeepAspectRatio(bool keepAspectRatio) {m_bKeepAspectRatio = keepAspectRatio; return this;}

	inline UString getImageResourceName() const {return m_sImageResourceName;}
	inline Vector2 getScale() const {return m_vScale;}

	// Overrides
	virtual CBaseUIImageButton *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIImageButton *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIImageButton *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIImageButton *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIImageButton *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIImageButton *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIImageButton *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIImageButton *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIImageButton *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIImageButton *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIImageButton *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIImageButton *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIImageButton *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIImageButton *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIImageButton *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIImageButton *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIImageButton *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIImageButton *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIImageButton *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIImageButton *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIImageButton *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIImageButton *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIImageButton *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIImageButton *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIImageButton *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIImageButton *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIImageButton *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIImageButton *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIImageButton *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIImageButton *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIImageButton *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIImageButton *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIImageButton *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIImageButton *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIImageButton *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIImageButton *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIImageButton *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIImageButton *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIImageButton *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIImageButton *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIImageButton *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIImageButton *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIImageButton *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIImageButton *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIImageButton *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIImageButton *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIImageButton *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIImageButton *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIImageButton *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIImageButton *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


protected:
	UString m_sImageResourceName;

	float m_fRot;
	Vector2 m_vScale;
	bool m_bScaleToFit;
	bool m_bKeepAspectRatio;
};

#endif
