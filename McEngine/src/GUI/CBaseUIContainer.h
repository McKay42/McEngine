//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		a container for UI elements
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUICONTAINER_H
#define CBASEUICONTAINER_H

#include "CBaseUIElement.h"

class CBaseUIContainer : public CBaseUIElement
{
public:
	CBaseUIContainer(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIContainer();
	void clear();
	void empty();

	void draw_debug(Graphics *g);
	virtual void draw(Graphics *g);
	virtual void update();

	void onKeyUp(KeyboardEvent &e);
	void onKeyDown(KeyboardEvent &e);
	void onChar(KeyboardEvent &e);

	CBaseUIContainer *addBaseUIElement(CBaseUIElement *element, float xPos, float yPos);
	CBaseUIContainer *addBaseUIElement(CBaseUIElement *element);
	CBaseUIContainer *addBaseUIElementBack(CBaseUIElement *element, float xPos, float yPos);
	CBaseUIContainer *addBaseUIElementBack(CBaseUIElement *element);

	CBaseUIContainer *insertBaseUIElement(CBaseUIElement *element, CBaseUIElement *index);
	CBaseUIContainer *insertBaseUIElementBack(CBaseUIElement *element, CBaseUIElement *index);

	CBaseUIContainer *removeBaseUIElement(CBaseUIElement *element);
	CBaseUIContainer *deleteBaseUIElement(CBaseUIElement *element);

	CBaseUIElement *getBaseUIElement(UString name);
	inline std::vector<CBaseUIElement*> getAllBaseUIElements(){return m_vElements;}
	inline std::vector<CBaseUIElement*> *getAllBaseUIElementsPointer(){return &m_vElements;}

	virtual void onMoved() {update_pos();}
	virtual void onResized() {update_pos();}

	virtual bool isBusy();
	virtual bool isActive();

	void onMouseDownOutside();

	virtual void onFocusStolen();
	virtual void onEnabled();
	virtual void onDisabled();

	void update_pos();
	void update_pos(CBaseUIElement *element);

	// Overrides
	virtual CBaseUIContainer *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIContainer *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIContainer *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIContainer *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIContainer *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainer *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainer *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainer *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIContainer *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIContainer *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIContainer *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIContainer *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIContainer *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIContainer *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIContainer *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIContainer *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIContainer *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainer *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIContainer *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIContainer *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIContainer *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainer *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainer *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainer *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIContainer *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIContainer *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIContainer *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIContainer *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIContainer *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIContainer *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIContainer *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIContainer *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIContainer *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIContainer *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIContainer *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIContainer *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIContainer *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIContainer *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIContainer *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIContainer *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIContainer *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIContainer *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIContainer *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIContainer *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIContainer *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIContainer *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIContainer *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIContainer *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIContainer *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIContainer *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

protected:
	std::vector<CBaseUIElement*> m_vElements;
};

#endif
