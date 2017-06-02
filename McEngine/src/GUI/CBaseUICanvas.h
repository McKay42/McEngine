/*
 * CBaseUICanvas.h
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#ifndef GUI_WINDOWS_CBASEUICANVAS_H_
#define GUI_WINDOWS_CBASEUICANVAS_H_

#include "cbase.h"
#include "CBaseUIContainerBase.h"

/*
 * UI Canvas Container
 * Scales any slotted containers or elements by the size of the canvas, useful for resolution scaling
 * The size/position of UI elements slotted should 0.0 to 1.0 as a percentage of the total screen area
 * Set scaleByHeightOnly per element to avoid stretching/squashing on aspect ratio changes. Uses a 16:9 (Widescreen) aspect ratio for assumed desired width
 */

class CBaseUICanvas : public CBaseUIContainerBase
{
public:
	CBaseUICanvas(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUICanvas();

	// main
	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0));

	// Overrides
	virtual CBaseUICanvas *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUICanvas *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUICanvas *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUICanvas *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUICanvas *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUICanvas *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUICanvas *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUICanvas *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUICanvas *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUICanvas *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUICanvas *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUICanvas *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUICanvas *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUICanvas *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUICanvas *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUICanvas *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUICanvas *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUICanvas *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUICanvas *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUICanvas *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUICanvas *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUICanvas *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUICanvas *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUICanvas *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUICanvas *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUICanvas *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUICanvas *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUICanvas *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUICanvas *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUICanvas *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUICanvas *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUICanvas *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUICanvas *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUICanvas *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUICanvas *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUICanvas *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUICanvas *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUICanvas *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUICanvas *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUICanvas *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUICanvas *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUICanvas *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUICanvas *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUICanvas *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUICanvas *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUICanvas *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUICanvas *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUICanvas *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUICanvas *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUICanvas *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

	virtual CBaseUICanvas *addElement(CBaseUIElement *element) {CBaseUIContainerBase::addElement(element); return this;}
	virtual CBaseUICanvas *addElementBack(CBaseUIElement *element) {CBaseUIContainerBase::addElementBack(element); return this;}
	virtual CBaseUICanvas *insertElement(CBaseUIElement *element, CBaseUIElement *index) {CBaseUIContainerBase::insertElement(element, index); return this;}
	virtual CBaseUICanvas *insertElementBack(CBaseUIElement *element, CBaseUIElement *index) {CBaseUIContainerBase::insertElementBack(element, index); return this;}

protected:
	// events
	virtual void onMoved();
	virtual void onResized();
	virtual void updateLayout();
	virtual void updateElement(CBaseUIElement *element);
};

#endif /* GUI_WINDOWS_CBASEUICANVAS_H_ */
