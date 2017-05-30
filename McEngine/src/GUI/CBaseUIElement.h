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
	CBaseUIElement(float xPos, float yPos, float xSize, float ySize, UString name);
	virtual ~CBaseUIElement() {;}

	// main
	virtual void draw(Graphics *g) = 0;
	virtual void update();

	// keyboard input
	virtual void onKeyUp(KeyboardEvent &e) {;}
	virtual void onKeyDown(KeyboardEvent &e) {;}
	virtual void onChar(KeyboardEvent &e) {;}

	// setters
	virtual void setPos(float xPos, float yPos) {if (m_vPos.x != xPos || m_vPos.y != yPos) {m_vPos.x = xPos - m_vSize.x * m_vAnchor.x; m_vPos.y = yPos - m_vSize.y * m_vAnchor.y; onMoved();}}
	virtual void setPosX(float xPos) {if (m_vPos.x != xPos) {m_vPos.x = xPos - m_vSize.x * m_vAnchor.x; onMoved();}}
	virtual void setPosY(float yPos) {if (m_vPos.y != yPos) {m_vPos.y = yPos - m_vSize.y * m_vAnchor.y; onMoved();}}
	virtual void setPos(Vector2 position) {if (m_vPos != position) {m_vPos = position - m_vSize * m_vAnchor; onMoved();}}

	virtual void setPosAbsolute(float xPos, float yPos) {if (m_vPos.x != xPos || m_vPos.y != yPos) {m_vPos.x = xPos; m_vPos.y = yPos; onMoved();}}
	virtual void setPosAbsoluteX(float xPos) {if (m_vPos.x != xPos) {m_vPos.x = xPos; onMoved();}}
	virtual void setPosAbsoluteY(float yPos) {if (m_vPos.y != yPos) {m_vPos.y = yPos; onMoved();}}
	virtual void setPosAbsolute(Vector2 position) {if (m_vPos != position) {m_vPos = position; onMoved();}}

	virtual void setRelPos(float xPos, float yPos) {if (m_vmPos.x != xPos || m_vmPos.y != yPos) {m_vmPos.x = xPos - m_vSize.x * m_vAnchor.x; m_vmPos.y = yPos - m_vSize.y * m_vAnchor.y; updateLayout();}}
	virtual void setRelPosX(float xPos) {if (m_vmPos.x != xPos) {m_vmPos.x = xPos - m_vSize.x * m_vAnchor.x; updateLayout();}}
	virtual void setRelPosY(float yPos) {if (m_vmPos.y != yPos) {m_vmPos.y = yPos - m_vSize.x * m_vAnchor.y; updateLayout();}}
	virtual void setRelPos(Vector2 position) {if (m_vmPos != position) {m_vmPos = position - m_vSize * m_vAnchor; updateLayout();}}

	virtual void setRelPosAbsolute(float xPos, float yPos) {if (m_vmPos.x != xPos || m_vmPos.y != yPos) {m_vmPos.x = xPos; m_vmPos.y = yPos; updateLayout();}}
	virtual void setRelPosAbsoluteX(float xPos) {if (m_vmPos.x != xPos) {m_vmPos.x = xPos; updateLayout();}}
	virtual void setRelPosAbsoluteY(float yPos) {if (m_vmPos.y != yPos) {m_vmPos.y = yPos; updateLayout();}}
	virtual void setRelPosAbsolute(Vector2 position) {if (m_vmPos != position) {m_vmPos = position; updateLayout();}}

	virtual void setSize(float xSize, float ySize) {if (m_vSize.x != xSize || m_vSize.y != ySize) {m_vPos.x += (m_vSize.x - xSize) * m_vAnchor.x; m_vPos.y += (m_vSize.y - ySize) * m_vAnchor.y; m_vSize.x = xSize; m_vSize.y = ySize; onResized(); onMoved();}}
	virtual void setSizeX(float xSize) {if (m_vSize.x != xSize) {m_vPos.x += (m_vSize.x - xSize) * m_vAnchor.x; m_vSize.x = xSize; onResized(); onMoved();}}
	virtual void setSizeY(float ySize) {if (m_vSize.y != ySize) {m_vPos.y += (m_vSize.y - ySize) * m_vAnchor.y; m_vSize.y = ySize; onResized(); onMoved();}}
	virtual void setSize(Vector2 size) {if (m_vSize != size) {m_vPos += (m_vSize - size) * m_vAnchor; m_vSize = size; onResized(); onMoved();}}

	virtual void setSizeAbsolute(float xSize, float ySize) {if (m_vSize.x != xSize || m_vSize.y != ySize) {m_vSize.x = xSize; m_vSize.y = ySize; onResized();}}
	virtual void setSizeAbsoluteX(float xSize) {if (m_vSize.x != xSize) {m_vSize.x = xSize; onResized();}}
	virtual void setSizeAbsoluteY(float ySize) {if (m_vSize.y != ySize) {m_vSize.y = ySize; onResized();}}
	virtual void setSizeAbsolute(Vector2 size) {if (m_vSize != size) {m_vSize = size; onResized();}}

	virtual void setRelSize(float xSize, float ySize) {if(m_vmSize.x != xSize || m_vmSize.y != ySize) {m_vmPos.x += (m_vmSize.x - xSize) * m_vAnchor.x; m_vmPos.y += (m_vmSize.y - ySize) * m_vAnchor.y; m_vmSize.x = xSize; m_vmSize.y = ySize; updateLayout();}}
	virtual void setRelSizeX(float xSize) {if (m_vmSize.x != xSize) {m_vmPos.x += (m_vmSize.x - xSize) * m_vAnchor.x; m_vmSize.x = xSize; updateLayout();}}
	virtual void setRelSizeY(float ySize) {if (m_vmSize.y != ySize) {m_vmPos.y += (m_vmSize.y - ySize) * m_vAnchor.y; m_vmSize.y = ySize; updateLayout();}}
	virtual void setRelSize(Vector2 size) {if (m_vmSize != size) {m_vmPos += (m_vmSize - size) * m_vAnchor; m_vmSize = size; updateLayout();}}

	virtual void setRelSizeAbsolute(float xSize, float ySize) {if (m_vmSize.x != xSize || m_vmSize.y != ySize) {m_vmSize.x = xSize; m_vmSize.y = ySize; updateLayout();}}
	virtual void setRelSizeAbsoluteX(float xSize) {if (m_vmSize.x != xSize) {m_vmSize.x = xSize; updateLayout();}}
	virtual void setRelSizeAbsoluteY(float ySize) {if (m_vmSize.y != ySize) {m_vmSize.y = ySize; updateLayout();}}
	virtual void setRelSizeAbsolute(Vector2 size) {if (m_vmSize != size) {m_vmSize = size; updateLayout();}}

	virtual void setAnchor(float xAnchor, float yAnchor) {if (m_vAnchor.x != xAnchor || m_vAnchor.y != yAnchor){m_vmPos.x -= m_vmSize.x * (xAnchor - m_vAnchor.x); m_vmPos.y -= m_vmSize.y * (yAnchor - m_vAnchor.y); m_vPos.x -= m_vSize.x * (xAnchor - m_vAnchor.x); m_vPos.y -= m_vSize.y * (yAnchor - m_vAnchor.y); m_vAnchor.x = xAnchor; m_vAnchor.y = yAnchor; if (m_parent != nullptr) updateLayout(); onMoved();}}
	virtual void setAnchorX(float xAnchor) {if (m_vAnchor.x != xAnchor){m_vmPos.x -= m_vmSize.x * (xAnchor - m_vAnchor.x); m_vPos.x -= m_vSize.x * (xAnchor - m_vAnchor.x); m_vAnchor.x = xAnchor; if (m_parent != nullptr) updateLayout(); onMoved();}}
	virtual void setAnchorY(float yAnchor) {if (m_vAnchor.y != yAnchor){m_vmPos.y -= m_vmSize.y * (yAnchor - m_vAnchor.y); m_vPos.y -= m_vSize.y * (yAnchor - m_vAnchor.y); m_vAnchor.y = yAnchor; if (m_parent != nullptr) updateLayout(); onMoved();}}
	virtual void setAnchor(Vector2 anchor) {if (m_vAnchor != anchor){m_vmPos -= m_vmSize * (anchor - m_vAnchor); m_vPos -= m_vSize * (anchor - m_vAnchor); m_vAnchor = anchor; if (m_parent != nullptr) updateLayout(); onMoved();}}

	virtual void setAnchorAbsolute(float xAnchor, float yAnchor) {if (m_vAnchor.x != xAnchor || m_vAnchor.y != yAnchor){m_vAnchor.x = xAnchor, m_vAnchor.y = yAnchor;}}
	virtual void setAnchorAbsoluteX(float xAnchor) {if (m_vAnchor.x != xAnchor) {m_vAnchor.x = xAnchor;}}
	virtual void setAnchorAbsoluteY(float yAnchor) {if (m_vAnchor.y != yAnchor) {m_vAnchor.y = yAnchor;}}
	virtual void setAnchorAbsolute(Vector2 anchor) {if (m_vAnchor != anchor) {m_vAnchor = anchor;}}

	virtual void setVisible(bool visible) {m_bVisible = visible;}
	virtual void setActive(bool active) {m_bActive = active;}
	virtual void setKeepActive(bool keepActive) {m_bKeepActive = keepActive;}
	virtual void setDrawManually(bool drawManually) {m_bDrawManually = drawManually;}
	virtual void setPositionManually(bool positionManually) {m_bPositionManually = positionManually;}
	virtual void setEnabled(bool enabled) {if (enabled != m_bEnabled) {m_bEnabled = enabled; if (m_bEnabled) {onEnabled();} else {onDisabled();}}}
	virtual void setBusy(bool busy) {m_bBusy = busy;}
	virtual void setName(UString name) {m_sName = name;}
	virtual void setParent(CBaseUIElement *parent) {m_parent = parent;}

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
