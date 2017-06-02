/*
 * CBaseUIContainerHorizontalBox.h
 *
 *  Created on: Jun 1, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERHORIZONTALBOX_H_
#define GUI_CBASEUICONTAINERHORIZONTALBOX_H_

#include "CBaseUIContainerBase.h"

class CBaseUIContainerHorizontalBox : public CBaseUIContainerBase
{
public:
	CBaseUIContainerHorizontalBox();
	virtual ~CBaseUIContainerHorizontalBox();

	// Overrides
	virtual CBaseUIContainerHorizontalBox *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIContainerHorizontalBox *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIContainerHorizontalBox *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainerHorizontalBox *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIContainerHorizontalBox *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIContainerHorizontalBox *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIContainerHorizontalBox *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIContainerHorizontalBox *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIContainerHorizontalBox *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainerHorizontalBox *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIContainerHorizontalBox *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIContainerHorizontalBox *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainerHorizontalBox *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIContainerHorizontalBox *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIContainerHorizontalBox *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIContainerHorizontalBox *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIContainerHorizontalBox *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIContainerHorizontalBox *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIContainerHorizontalBox *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIContainerHorizontalBox *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIContainerHorizontalBox *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIContainerHorizontalBox *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIContainerHorizontalBox *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIContainerHorizontalBox *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIContainerHorizontalBox *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIContainerHorizontalBox *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIContainerHorizontalBox *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIContainerHorizontalBox *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIContainerHorizontalBox *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIContainerHorizontalBox *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIContainerHorizontalBox *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

	virtual CBaseUIContainerHorizontalBox *addElement(CBaseUIElement *element) {CBaseUIContainerBase::addElement(element); return this;}
	virtual CBaseUIContainerHorizontalBox *addElementBack(CBaseUIElement *element) {CBaseUIContainerBase::addElementBack(element); return this;}
	virtual CBaseUIContainerHorizontalBox *insertElement(CBaseUIElement *element, CBaseUIElement *index) {CBaseUIContainerBase::insertElement(element, index); return this;}
	virtual CBaseUIContainerHorizontalBox *insertElementBack(CBaseUIElement *element, CBaseUIElement *index) {CBaseUIContainerBase::insertElementBack(element, index); return this;}
};

#endif /* GUI_CBASEUICONTAINERHORIZONTALBOX_H_ */
