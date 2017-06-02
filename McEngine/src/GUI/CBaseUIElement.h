//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		the base class for all UI Elements
//
// $NoKeywords: $buie
//===============================================================================//

#ifndef CBASEUIELEMENT_H
#define CBASEUIELEMENT_H

#include "cbase.h"
#include "KeyboardListener.h"

class CBaseUIElement : public KeyboardListener
{
public:
	CBaseUIElement(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUIElement() {;}

	// main
	virtual void draw(Graphics *g) = 0;
	virtual void update();

	// keyboard input
	virtual void onKeyUp(KeyboardEvent &e) {;}
	virtual void onKeyDown(KeyboardEvent &e) {;}
	virtual void onChar(KeyboardEvent &e) {;}

	// setters
	virtual CBaseUIElement *setPos(float xPos, float yPos) {if (m_vPos.x != xPos || m_vPos.y != yPos) {m_vPos.x = xPos - m_vSize.x * m_vAnchor.x; m_vPos.y = yPos - m_vSize.y * m_vAnchor.y; onMoved();} return this;}
	virtual CBaseUIElement *setPosX(float xPos) {if (m_vPos.x != xPos) {m_vPos.x = xPos - m_vSize.x * m_vAnchor.x; onMoved();} return this;}
	virtual CBaseUIElement *setPosY(float yPos) {if (m_vPos.y != yPos) {m_vPos.y = yPos - m_vSize.y * m_vAnchor.y; onMoved();} return this;}
	virtual CBaseUIElement *setPos(Vector2 position) {if (m_vPos != position) {m_vPos = position - m_vSize * m_vAnchor; onMoved();} return this;}

	virtual CBaseUIElement *setPosAbsolute(float xPos, float yPos) {if (m_vPos.x != xPos || m_vPos.y != yPos) {m_vPos.x = xPos; m_vPos.y = yPos; onMoved();} return this;}
	virtual CBaseUIElement *setPosAbsoluteX(float xPos) {if (m_vPos.x != xPos) {m_vPos.x = xPos; onMoved();} return this;}
	virtual CBaseUIElement *setPosAbsoluteY(float yPos) {if (m_vPos.y != yPos) {m_vPos.y = yPos; onMoved();} return this;}
	virtual CBaseUIElement *setPosAbsolute(Vector2 position) {if (m_vPos != position) {m_vPos = position; onMoved();} return this;}

	virtual CBaseUIElement *setRelPos(float xPos, float yPos) {if (m_vmPos.x != xPos || m_vmPos.y != yPos) {m_vmPos.x = xPos - m_vSize.x * m_vAnchor.x; m_vmPos.y = yPos - m_vSize.y * m_vAnchor.y; updateLayout();} return this;}
	virtual CBaseUIElement *setRelPosX(float xPos) {if (m_vmPos.x != xPos) {m_vmPos.x = xPos - m_vSize.x * m_vAnchor.x; updateLayout();} return this;}
	virtual CBaseUIElement *setRelPosY(float yPos) {if (m_vmPos.y != yPos) {m_vmPos.y = yPos - m_vSize.x * m_vAnchor.y; updateLayout();} return this;}
	virtual CBaseUIElement *setRelPos(Vector2 position) {if (m_vmPos != position) {m_vmPos = position - m_vSize * m_vAnchor; updateLayout();} return this;}

	virtual CBaseUIElement *setRelPosAbsolute(float xPos, float yPos) {if (m_vmPos.x != xPos || m_vmPos.y != yPos) {m_vmPos.x = xPos; m_vmPos.y = yPos; updateLayout();} return this;}
	virtual CBaseUIElement *setRelPosAbsoluteX(float xPos) {if (m_vmPos.x != xPos) {m_vmPos.x = xPos; updateLayout();} return this;}
	virtual CBaseUIElement *setRelPosAbsoluteY(float yPos) {if (m_vmPos.y != yPos) {m_vmPos.y = yPos; updateLayout();} return this;}
	virtual CBaseUIElement *setRelPosAbsolute(Vector2 position) {if (m_vmPos != position) {m_vmPos = position; updateLayout();} return this;}

	virtual CBaseUIElement *setSize(float xSize, float ySize) {if (m_vSize.x != xSize || m_vSize.y != ySize) {m_vPos.x += (m_vSize.x - xSize) * m_vAnchor.x; m_vPos.y += (m_vSize.y - ySize) * m_vAnchor.y; m_vSize.x = xSize; m_vSize.y = ySize; onResized(); onMoved();} return this;}
	virtual CBaseUIElement *setSizeX(float xSize) {if (m_vSize.x != xSize) {m_vPos.x += (m_vSize.x - xSize) * m_vAnchor.x; m_vSize.x = xSize; onResized(); onMoved();} return this;}
	virtual CBaseUIElement *setSizeY(float ySize) {if (m_vSize.y != ySize) {m_vPos.y += (m_vSize.y - ySize) * m_vAnchor.y; m_vSize.y = ySize; onResized(); onMoved();} return this;}
	virtual CBaseUIElement *setSize(Vector2 size) {if (m_vSize != size) {m_vPos += (m_vSize - size) * m_vAnchor; m_vSize = size; onResized(); onMoved();} return this;}

	virtual CBaseUIElement *setSizeAbsolute(float xSize, float ySize) {if (m_vSize.x != xSize || m_vSize.y != ySize) {m_vSize.x = xSize; m_vSize.y = ySize; onResized();} return this;}
	virtual CBaseUIElement *setSizeAbsoluteX(float xSize) {if (m_vSize.x != xSize) {m_vSize.x = xSize; onResized();} return this;}
	virtual CBaseUIElement *setSizeAbsoluteY(float ySize) {if (m_vSize.y != ySize) {m_vSize.y = ySize; onResized();} return this;}
	virtual CBaseUIElement *setSizeAbsolute(Vector2 size) {if (m_vSize != size) {m_vSize = size; onResized();} return this;}

	virtual CBaseUIElement *setRelSize(float xSize, float ySize) {if(m_vmSize.x != xSize || m_vmSize.y != ySize) {m_vmPos.x += (m_vmSize.x - xSize) * m_vAnchor.x; m_vmPos.y += (m_vmSize.y - ySize) * m_vAnchor.y; m_vmSize.x = xSize; m_vmSize.y = ySize; updateLayout();} return this;}
	virtual CBaseUIElement *setRelSizeX(float xSize) {if (m_vmSize.x != xSize) {m_vmPos.x += (m_vmSize.x - xSize) * m_vAnchor.x; m_vmSize.x = xSize; updateLayout();} return this;}
	virtual CBaseUIElement *setRelSizeY(float ySize) {if (m_vmSize.y != ySize) {m_vmPos.y += (m_vmSize.y - ySize) * m_vAnchor.y; m_vmSize.y = ySize; updateLayout();} return this;}
	virtual CBaseUIElement *setRelSize(Vector2 size) {if (m_vmSize != size) {m_vmPos += (m_vmSize - size) * m_vAnchor; m_vmSize = size; updateLayout();} return this;}

	virtual CBaseUIElement *setRelSizeAbsolute(float xSize, float ySize) {if (m_vmSize.x != xSize || m_vmSize.y != ySize) {m_vmSize.x = xSize; m_vmSize.y = ySize; updateLayout();} return this;}
	virtual CBaseUIElement *setRelSizeAbsoluteX(float xSize) {if (m_vmSize.x != xSize) {m_vmSize.x = xSize; updateLayout();} return this;}
	virtual CBaseUIElement *setRelSizeAbsoluteY(float ySize) {if (m_vmSize.y != ySize) {m_vmSize.y = ySize; updateLayout();} return this;}
	virtual CBaseUIElement *setRelSizeAbsolute(Vector2 size) {if (m_vmSize != size) {m_vmSize = size; updateLayout();} return this;}

	virtual CBaseUIElement *setAnchor(float xAnchor, float yAnchor) {if (m_vAnchor.x != xAnchor || m_vAnchor.y != yAnchor){m_vmPos.x -= m_vmSize.x * (xAnchor - m_vAnchor.x); m_vmPos.y -= m_vmSize.y * (yAnchor - m_vAnchor.y); m_vPos.x -= m_vSize.x * (xAnchor - m_vAnchor.x); m_vPos.y -= m_vSize.y * (yAnchor - m_vAnchor.y); m_vAnchor.x = xAnchor; m_vAnchor.y = yAnchor; if (m_parent != nullptr) updateLayout(); onMoved();} return this;}
	virtual CBaseUIElement *setAnchorX(float xAnchor) {if (m_vAnchor.x != xAnchor){m_vmPos.x -= m_vmSize.x * (xAnchor - m_vAnchor.x); m_vPos.x -= m_vSize.x * (xAnchor - m_vAnchor.x); m_vAnchor.x = xAnchor; if (m_parent != nullptr) updateLayout(); onMoved();} return this;}
	virtual CBaseUIElement *setAnchorY(float yAnchor) {if (m_vAnchor.y != yAnchor){m_vmPos.y -= m_vmSize.y * (yAnchor - m_vAnchor.y); m_vPos.y -= m_vSize.y * (yAnchor - m_vAnchor.y); m_vAnchor.y = yAnchor; if (m_parent != nullptr) updateLayout(); onMoved();} return this;}
	virtual CBaseUIElement *setAnchor(Vector2 anchor) {if (m_vAnchor != anchor){m_vmPos -= m_vmSize * (anchor - m_vAnchor); m_vPos -= m_vSize * (anchor - m_vAnchor); m_vAnchor = anchor; if (m_parent != nullptr) updateLayout(); onMoved();} return this;}

	virtual CBaseUIElement *setAnchorAbsolute(float xAnchor, float yAnchor) {if (m_vAnchor.x != xAnchor || m_vAnchor.y != yAnchor){m_vAnchor.x = xAnchor, m_vAnchor.y = yAnchor;} return this;}
	virtual CBaseUIElement *setAnchorAbsoluteX(float xAnchor) {if (m_vAnchor.x != xAnchor) {m_vAnchor.x = xAnchor;} return this;}
	virtual CBaseUIElement *setAnchorAbsoluteY(float yAnchor) {if (m_vAnchor.y != yAnchor) {m_vAnchor.y = yAnchor;} return this;}
	virtual CBaseUIElement *setAnchorAbsolute(Vector2 anchor) {if (m_vAnchor != anchor) {m_vAnchor = anchor;} return this;}

	virtual CBaseUIElement *setVisible(bool visible) {m_bVisible = visible; return this;}
	virtual CBaseUIElement *setActive(bool active) {m_bActive = active; return this;}
	virtual CBaseUIElement *setKeepActive(bool keepActive) {m_bKeepActive = keepActive; return this;}
	virtual CBaseUIElement *setDrawManually(bool drawManually) {m_bDrawManually = drawManually; return this;}
	virtual CBaseUIElement *setPositionManually(bool positionManually) {m_bPositionManually = positionManually; return this;}
	virtual CBaseUIElement *setEnabled(bool enabled) {if (enabled != m_bEnabled) {m_bEnabled = enabled; if (m_bEnabled) {onEnabled();} else {onDisabled();}} return this;}
	virtual CBaseUIElement *setBusy(bool busy) {m_bBusy = busy; return this;}
	virtual CBaseUIElement *setName(UString name) {m_sName = name; return this;}
	virtual CBaseUIElement *setParent(CBaseUIElement *parent) {m_parent = parent; return this;}
	virtual CBaseUIElement *setScaleByHeightOnly(bool scaleByHeightOnly) {m_bScaleByHeightOnly = scaleByHeightOnly; return this;}

	// getters
	inline const Vector2& getPos() const {return m_vPos;}
	inline const Vector2& getSize() const {return m_vSize;}
	inline UString getName() const {return m_sName;}
	inline const Vector2& getRelPos() const {return m_vmPos;}
	inline const Vector2& getRelSize() const {return m_vmSize;}
	inline const Vector2& getAnchor() const {return m_vAnchor;}
	inline CBaseUIElement *getParent() const {return m_parent;}

	virtual bool isActive() {return m_bActive || isBusy();}
	virtual bool isVisible() {return m_bVisible;}
	virtual bool isEnabled() {return m_bEnabled;}
	virtual bool isBusy() {return m_bBusy && isVisible();}
	virtual bool isDrawnManually() {return m_bDrawManually;}
	virtual bool isPositionedManually() {return m_bPositionManually;}
	virtual bool isMouseInside() {return m_bMouseInside && isVisible();}
	virtual bool isScaledByHeightOnly() {return m_bScaleByHeightOnly;}

	// actions
	void stealFocus() {m_bMouseInsideCheck = true; m_bActive = false; onFocusStolen();}
	virtual void updateLayout() {if(m_parent != nullptr) m_parent->updateLayout();}

protected:
	// events
	virtual void onResized() {;}
	virtual void onMoved() {;}

	virtual void onFocusStolen() {;}
	virtual void onEnabled() {;}
	virtual void onDisabled() {;}

	virtual void onMouseInside() {;}
	virtual void onMouseOutside() {;}
	virtual void onMouseDownInside() {;}
	virtual void onMouseDownOutside() {;}
	virtual void onMouseUpInside() {;}
	virtual void onMouseUpOutside() {;}

	// vars
	UString m_sName;
	CBaseUIElement *m_parent;

	// attributes
	bool m_bVisible;
	bool m_bActive;			// we are doing something, e.g. textbox is blinking and ready to receive input
	bool m_bBusy;			// we demand the focus to be kept on us, e.g. click-drag scrolling in a scrollview
	bool m_bEnabled;

	bool m_bKeepActive;		// once clicked, don't lose m_bActive, we have to manually release it (e.g. textbox)
	bool m_bDrawManually;
	bool m_bPositionManually;
	bool m_bMouseInside;

	// container options
	bool m_bScaleByHeightOnly;

	// position and size
	Vector2 m_vPos;
	Vector2 m_vmPos;
	Vector2 m_vSize;
	Vector2 m_vmSize;
	Vector2 m_vAnchor;		// the point of transformation

private:
	bool m_bMouseInsideCheck;
	bool m_bMouseUpCheck;
};

#endif
