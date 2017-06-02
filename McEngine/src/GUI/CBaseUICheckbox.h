//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple checkbox
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUICHECKBOX_H
#define CBASEUICHECKBOX_H

#include "CBaseUIButton.h"

class CBaseUICheckbox : public CBaseUIButton
{
public:
	CBaseUICheckbox(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="", UString text="");
	virtual ~CBaseUICheckbox() {;}

	virtual void draw(Graphics *g);

	inline float getBlockSize() {return m_vSize.y/2;}
	inline float getBlockBorder() {return m_vSize.y/4;}
	inline bool isChecked() const {return m_bChecked;}

	CBaseUICheckbox *setChecked(bool checked, bool fireChangeEvent = true);
	CBaseUICheckbox *setSizeToContent(int horizontalBorderSize = 1, int verticalBorderSize = 1);
	CBaseUICheckbox *setWidthToContent(int horizontalBorderSize = 1);

	typedef fastdelegate::FastDelegate1<CBaseUICheckbox*> CheckboxChangeCallback;
	CBaseUICheckbox *setChangeCallback( CheckboxChangeCallback clickCallback ) {m_changeCallback = clickCallback; return this;}

	// Overrides
	virtual CBaseUICheckbox *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUICheckbox *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUICheckbox *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUICheckbox *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUICheckbox *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUICheckbox *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUICheckbox *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUICheckbox *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUICheckbox *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUICheckbox *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUICheckbox *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUICheckbox *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUICheckbox *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUICheckbox *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUICheckbox *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUICheckbox *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUICheckbox *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUICheckbox *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUICheckbox *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUICheckbox *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUICheckbox *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUICheckbox *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUICheckbox *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUICheckbox *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUICheckbox *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUICheckbox *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUICheckbox *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUICheckbox *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUICheckbox *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUICheckbox *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUICheckbox *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUICheckbox *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUICheckbox *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUICheckbox *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUICheckbox *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUICheckbox *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUICheckbox *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUICheckbox *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUICheckbox *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUICheckbox *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUICheckbox *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUICheckbox *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUICheckbox *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUICheckbox *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUICheckbox *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUICheckbox *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUICheckbox *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUICheckbox *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUICheckbox *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUICheckbox *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

protected:
	void onPressed();

	bool m_bChecked;
	CheckboxChangeCallback m_changeCallback;
};


#endif
