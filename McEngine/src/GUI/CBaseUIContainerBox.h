/*
 * CBaseUIContainerBox.h
 *
 *  Created on: Jun 1, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERBOX_H_
#define GUI_CBASEUICONTAINERBOX_H_

#include "CBaseUIContainerBase.h"

class CBaseUIContainerBox : public CBaseUIContainerBase
{
public:
	CBaseUIContainerBox(float xPos=0, float yPos=0, UString name="");
	virtual ~CBaseUIContainerBox();

	// Overrides
	virtual CBaseUIContainerBox *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIContainerBox *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIContainerBox *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIContainerBox *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIContainerBox *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainerBox *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainerBox *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainerBox *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIContainerBox *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIContainerBox *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIContainerBox *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIContainerBox *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIContainerBox *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainerBox *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainerBox *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainerBox *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIContainerBox *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBox *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIContainerBox *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIContainerBox *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIContainerBox *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBox *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainerBox *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainerBox *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIContainerBox *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBox *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIContainerBox *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIContainerBox *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIContainerBox *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBox *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainerBox *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainerBox *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIContainerBox *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIContainerBox *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIContainerBox *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIContainerBox *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIContainerBox *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIContainerBox *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIContainerBox *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIContainerBox *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIContainerBox *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIContainerBox *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIContainerBox *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIContainerBox *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIContainerBox *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIContainerBox *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIContainerBox *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIContainerBox *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIContainerBox *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIContainerBox *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

	virtual CBaseUIContainerBox *addElement(CBaseUIElement *element) {CBaseUIContainerBase::addElement(element); return this;}
	virtual CBaseUIContainerBox *addElementBack(CBaseUIElement *element) {CBaseUIContainerBase::addElementBack(element); return this;}
	virtual CBaseUIContainerBox *insertElement(CBaseUIElement *element, CBaseUIElement *index) {CBaseUIContainerBase::insertElement(element, index); return this;}
	virtual CBaseUIContainerBox *insertElementBack(CBaseUIElement *element, CBaseUIElement *index) {CBaseUIContainerBase::insertElement(element, index); return this;}

protected:
	virtual void updateLayout();
	virtual void updateElement(CBaseUIElement *element);


};

#endif /* GUI_CBASEUICONTAINERBOX_H_ */
