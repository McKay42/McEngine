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

	ELEMENT_BODY(CBaseUIWindow)

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
