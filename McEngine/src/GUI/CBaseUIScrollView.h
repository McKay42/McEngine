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
	CBaseUIScrollView(float xPos, float yPos, float xSize, float ySize, UString name);
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
	void setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground;}
	void setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame;}
	void setDrawScrollbars(bool drawScrollbars) {m_bDrawScrollbars = drawScrollbars;}

	void setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor;}
	void setFrameColor(Color frameColor) {m_frameColor = frameColor;}
	void setFrameBrightColor(Color frameBrightColor) {m_frameBrightColor = frameBrightColor;}
	void setFrameDarkColor(Color frameDarkColor) {m_frameDarkColor = frameDarkColor;}
	void setScrollbarColor(Color scrollbarColor) {m_scrollbarColor = scrollbarColor;}

	void setHorizontalScrolling(bool horizontalScrolling) {m_bHorizontalScrolling = horizontalScrolling;}
	void setVerticalScrolling(bool verticalScrolling) {m_bVerticalScrolling = verticalScrolling;}
	void setScrollSizeToContent(int border = 5);
	void setScrollResistance(int scrollResistanceInPixels) {m_iScrollResistance = scrollResistanceInPixels;}

	void setBlockScrolling(bool block) {m_bBlockScrolling = block;} // means: disable scrolling, not scrolling in 'blocks'

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

	int m_iScrollResistance;
};

#endif
