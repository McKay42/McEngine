//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		smooth kinetic scrolling container
//
// $NoKeywords: $
//===============================================================================//

// TODO: refactor the spaghetti parts, this can be done way more elegantly

#ifndef CBASEUISCROLLVIEW_H
#define CBASEUISCROLLVIEW_H

#include "CBaseUIElement.h"

class CBaseUIContainer;

class CBaseUIScrollView : public CBaseUIElement
{
public:
	CBaseUIScrollView(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIScrollView();

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onKeyUp(KeyboardEvent &e);
	virtual void onKeyDown(KeyboardEvent &e);
	virtual void onChar(KeyboardEvent &e);

	void clear();

	// scrolling
	void scrollY(int delta, bool animated = true);
	void scrollX(int delta, bool animated = true);
	void scrollToY(int scrollPosY, bool animated = true);
	void scrollToX(int scrollPosX, bool animated = true);
	void scrollToElement(CBaseUIElement *element, int xOffset = 0, int yOffset = 0);

	void scrollToRight();
	void scrollToBottom();
	void scrollToTop();

	// set
	CBaseUIScrollView *setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground; return this;}
	CBaseUIScrollView *setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame; return this;}
	CBaseUIScrollView *setDrawScrollbars(bool drawScrollbars) {m_bDrawScrollbars = drawScrollbars; return this;}

	CBaseUIScrollView *setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor; return this;}
	CBaseUIScrollView *setFrameColor(Color frameColor) {m_frameColor = frameColor; return this;}
	CBaseUIScrollView *setFrameBrightColor(Color frameBrightColor) {m_frameBrightColor = frameBrightColor; return this;}
	CBaseUIScrollView *setFrameDarkColor(Color frameDarkColor) {m_frameDarkColor = frameDarkColor; return this;}
	CBaseUIScrollView *setScrollbarColor(Color scrollbarColor) {m_scrollbarColor = scrollbarColor; return this;}

	CBaseUIScrollView *setHorizontalScrolling(bool horizontalScrolling) {m_bHorizontalScrolling = horizontalScrolling; return this;}
	CBaseUIScrollView *setVerticalScrolling(bool verticalScrolling) {m_bVerticalScrolling = verticalScrolling; return this;}
	CBaseUIScrollView *setScrollSizeToContent(int border = 5);
	CBaseUIScrollView *setScrollResistance(int scrollResistanceInPixels) {m_iScrollResistance = scrollResistanceInPixels; return this;}

	CBaseUIScrollView *setBlockScrolling(bool block) {m_bBlockScrolling = block; return this;} // means: disable scrolling, not scrolling in 'blocks'

	// get
	inline CBaseUIContainer *getContainer() {return m_container;}
	inline float getScrollPosY() {return m_vScrollPos.y;}
	inline float getScrollPosX() {return m_vScrollPos.x;}
	inline Vector2 const getScrollSize() {return m_vScrollSize;}
	Vector2 getVelocity();

	// is
	inline bool isScrolling() {return m_bScrolling;}
	bool isBusy();

	// events
	void onResized();
	void onMouseDownOutside();
	void onMouseDownInside();
	void onMouseUpInside();
	void onMouseUpOutside();

	void onFocusStolen();
	void onEnabled();
	void onDisabled();

	// Overrides
	virtual CBaseUIScrollView *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIScrollView *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIScrollView *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIScrollView *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIScrollView *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIScrollView *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIScrollView *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIScrollView *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIScrollView *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIScrollView *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIScrollView *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIScrollView *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIScrollView *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIScrollView *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIScrollView *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIScrollView *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIScrollView *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIScrollView *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIScrollView *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIScrollView *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIScrollView *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIScrollView *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIScrollView *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIScrollView *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIScrollView *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIScrollView *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIScrollView *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIScrollView *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIScrollView *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIScrollView *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIScrollView *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIScrollView *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIScrollView *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIScrollView *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIScrollView *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIScrollView *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIScrollView *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIScrollView *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIScrollView *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIScrollView *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIScrollView *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIScrollView *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIScrollView *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIScrollView *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIScrollView *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIScrollView *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIScrollView *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIScrollView *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIScrollView *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIScrollView *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


protected:
	virtual void onMoved();

private:
	void updateClipping();
	void updateScrollbars();

	// main container
	CBaseUIContainer *m_container;

	// vars
	bool m_bDrawFrame;
	bool m_bDrawBackground;
	bool m_bDrawScrollbars;

	Color m_backgroundColor;
	Color m_frameColor;
	Color m_frameBrightColor;
	Color m_frameDarkColor;
	Color m_scrollbarColor;

	Vector2 m_vScrollPos;
	Vector2 m_vScrollPosBackup;
	Vector2 m_vMouseBackup;

	Rect m_verticalScrollbar;
	Rect m_horizontalScrollbar;

	// scroll logic
	bool m_bScrolling;
	bool m_bScrollbarScrolling;
	bool m_bScrollbarIsVerticalScrolling;
	bool m_bBlockScrolling;
	bool m_bHorizontalScrolling;
	bool m_bVerticalScrolling;
	Vector2 m_vScrollSize;
	Vector2 m_vMouseBackup2;
	Vector2 m_vMouseBackup3;
	Vector2 m_vVelocity;
	Vector2 m_vKineticAverage;

	bool m_bAutoScrollingX;
	bool m_bAutoScrollingY;
	int m_iPrevScrollDeltaX,m_iPrevScrollDeltaY;

	bool m_bScrollResistanceCheck;
	int m_iScrollResistance;
};

#endif
