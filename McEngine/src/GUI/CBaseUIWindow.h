//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		base class for windows
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUIWINDOW_H
#define CBASEUIWINDOW_H

#include "CBaseUIElement.h"

class CBaseUIButton;
class CBaseUIContainer;
class CBaseUIBoxShadow;

class RenderTarget;

class CBaseUIWindow : public CBaseUIElement
{
public:
	CBaseUIWindow(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	~CBaseUIWindow();

	virtual void draw(Graphics *g);
	virtual void drawCustomContent(Graphics *g) {;}
	virtual void update();

	virtual void onKeyDown(KeyboardEvent &e);
	virtual void onKeyUp(KeyboardEvent &e);
	virtual void onChar(KeyboardEvent &e);

	// actions
	void close();
	void open();

	void minimize();

	// BETA: mimic native window
	CBaseUIWindow *enableCoherenceMode();

	// set
	CBaseUIWindow *setSizeToContent(int horizontalBorderSize = 1, int verticalBorderSize = 1);
	CBaseUIWindow *setTitleBarHeight(int height) {m_iTitleBarHeight = height; updateTitleBarMetrics(); return this;}
	CBaseUIWindow *setTitle(UString text);
	CBaseUIWindow *setTitleFont(McFont *titleFont) {m_titleFont = titleFont; updateTitleBarMetrics(); return this;}
	CBaseUIWindow *setResizeLimit(int maxWidth, int maxHeight) {m_vResizeLimit = Vector2(maxWidth, maxHeight); return this;}
	CBaseUIWindow *setResizeable(bool resizeable) {m_bResizeable = resizeable; return this;}
	CBaseUIWindow *setDrawTitleBarLine(bool drawTitleBarLine) {m_bDrawTitleBarLine = drawTitleBarLine; return this;}
	CBaseUIWindow *setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame; return this;}
	CBaseUIWindow *setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground; return this;}
	CBaseUIWindow *setRoundedRectangle(bool roundedRectangle) {m_bRoundedRectangle = roundedRectangle; return this;}

	CBaseUIWindow *setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor; return this;}
	CBaseUIWindow *setFrameColor(Color frameColor) {m_frameColor = frameColor; return this;}
	CBaseUIWindow *setFrameBrightColor(Color frameBrightColor) {m_frameBrightColor = frameBrightColor; return this;}
	CBaseUIWindow *setFrameDarkColor(Color frameDarkColor) {m_frameDarkColor = frameDarkColor; return this;}
	CBaseUIWindow *setTitleColor(Color titleColor) {m_titleColor = titleColor; return this;}

	// get
	virtual bool isBusy();
	virtual bool isActive();
	inline bool isMoving() const {return m_bMoving;}
	inline bool isResizing() const {return m_bResizing;}
	inline CBaseUIContainer *getContainer() const {return m_container;}
	inline CBaseUIContainer *getTitleBarContainer() const {return m_titleBarContainer;}
	inline int getTitleBarHeight() {return m_iTitleBarHeight;}

	// events
	virtual void onMouseDownInside();
	virtual void onMouseUpInside();
	virtual void onMouseUpOutside();

	virtual void onMoved();
	virtual void onResized();

	virtual void onResolutionChange(Vector2 newResolution);

	virtual void onEnabled();
	virtual void onDisabled();

	// Overrides
	virtual CBaseUIWindow *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIWindow *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIWindow *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIWindow *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIWindow *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIWindow *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIWindow *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIWindow *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIWindow *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIWindow *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIWindow *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIWindow *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIWindow *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIWindow *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIWindow *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIWindow *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIWindow *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIWindow *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIWindow *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIWindow *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIWindow *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIWindow *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIWindow *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIWindow *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIWindow *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIWindow *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIWindow *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIWindow *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIWindow *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIWindow *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIWindow *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIWindow *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIWindow *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIWindow *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIWindow *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIWindow *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIWindow *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIWindow *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIWindow *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIWindow *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIWindow *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIWindow *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIWindow *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIWindow *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIWindow *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIWindow *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIWindow *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIWindow *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIWindow *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIWindow *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


protected:
	void updateTitleBarMetrics();
	void udpateResizeAndMoveLogic(bool captureMouse);
	void updateWindowLogic();

	virtual void onClosed();

	inline CBaseUIButton *getCloseButton() {return m_closeButton;}
	inline CBaseUIButton *getMinimizeButton() {return m_minimizeButton;}

private:
	// colors
	Color m_frameColor;
	Color m_frameBrightColor;
	Color m_frameDarkColor;
	Color m_backgroundColor;
	Color m_titleColor;

	// window properties
	bool m_bIsOpen;
	bool m_bAnimIn;
	bool m_bResizeable;
	bool m_bCoherenceMode;
	float m_fAnimation;

	bool m_bDrawFrame;
	bool m_bDrawBackground;
	bool m_bRoundedRectangle;

	// title bar
	bool m_bDrawTitleBarLine;
	CBaseUIContainer *m_titleBarContainer;
	McFont *m_titleFont;
	float m_fTitleFontWidth;
	float m_fTitleFontHeight;
	int m_iTitleBarHeight;
	UString m_sTitle;

	CBaseUIButton *m_closeButton;
	CBaseUIButton *m_minimizeButton;

	// main container
	CBaseUIContainer *m_container;

	// moving
	bool m_bMoving;
	Vector2 m_vMousePosBackup;
	Vector2 m_vLastPos;

	// resizing
	Vector2 m_vResizeLimit;
	bool m_bResizing;
	int m_iResizeType;
	Vector2 m_vLastSize;

	// test features
	RenderTarget *m_rt;
	CBaseUIBoxShadow *m_shadow;
};

#endif
