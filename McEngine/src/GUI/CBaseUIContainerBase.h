/*
 * CBaseUIContainerBase.h
 *
 *  Created on: May 31, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERBASE_H_
#define GUI_CBASEUICONTAINERBASE_H_

#include "CBaseUIElement.h"
#include "cbase.h"

class CBaseUIContainerBase : public CBaseUIElement
{
public:

	CBaseUIContainerBase(UString name="");
	virtual ~CBaseUIContainerBase();

	// container
	virtual CBaseUIContainerBase *addElement(CBaseUIElement *element);
	virtual CBaseUIContainerBase *addElementBack(CBaseUIElement *element);
	virtual CBaseUIContainerBase *insertElement(CBaseUIElement *element, CBaseUIElement *index);
	virtual CBaseUIContainerBase *insertElementBack(CBaseUIElement *element, CBaseUIElement *index);

	virtual void removeElement(CBaseUIElement *element);
	virtual void deleteElement(CBaseUIElement *element);

	CBaseUIElement *getElementByName(UString name);
	inline std::vector<CBaseUIElement*> getAllElements(){return m_vElements;}
	inline std::vector<CBaseUIElement*> *getAllElementsReference(){return &m_vElements;}

	virtual void draw(Graphics *g);
	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0)) {;}
	virtual void update();

	virtual void clear();
	virtual void empty();

	// Overrides
	virtual CBaseUIContainerBase *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIContainerBase *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIContainerBase *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIContainerBase *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIContainerBase *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainerBase *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainerBase *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainerBase *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIContainerBase *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIContainerBase *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIContainerBase *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIContainerBase *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIContainerBase *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainerBase *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainerBase *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainerBase *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIContainerBase *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBase *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIContainerBase *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIContainerBase *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIContainerBase *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBase *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainerBase *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainerBase *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIContainerBase *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBase *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIContainerBase *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIContainerBase *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIContainerBase *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainerBase *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainerBase *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainerBase *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIContainerBase *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIContainerBase *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIContainerBase *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIContainerBase *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIContainerBase *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIContainerBase *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIContainerBase *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIContainerBase *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIContainerBase *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIContainerBase *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIContainerBase *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIContainerBase *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIContainerBase *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIContainerBase *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIContainerBase *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIContainerBase *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIContainerBase *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIContainerBase *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

protected:
	// events
	virtual void updateElement(CBaseUIElement *element) {;}

	std::vector<CBaseUIElement *> m_vElements;
};

#endif /* GUI_CBASEUICONTAINERBASE_H_ */
