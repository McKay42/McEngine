/*
 * CBaseUICanvas.h
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#ifndef GUI_WINDOWS_CBASEUICANVAS_H_
#define GUI_WINDOWS_CBASEUICANVAS_H_

#include "cbase.h"
#include "Engine.h"

class CBaseUIElement;

/*
 * UI Canvas Container
 * Scales any slotted containers or elements by the size of the canvas, useful for resolution scaling
 * The size/position of UI elements slotted should 0.0 to 1.0 as a percentage of the total screen area
 * Set scaleByHeightOnly per element to avoid stretching/squashing on aspect ratio changes. Uses a 16:9 (Widescreen) aspect ratio for assumed desired width
 */
struct Slot{
	CBaseUIElement *element;
	bool scaleByHeightOnly=false;
};

class CBaseUICanvas {
public:
	CBaseUICanvas(float xPos, float yPos, float xSize, float ySize);
	virtual ~CBaseUICanvas();

	// main
	virtual void draw(Graphics *g);
	virtual void update();

	// container
	void addSlot(Slot *slot);
	void addSlot(CBaseUIElement *element, bool scaleByHeightOnly=false);
	void addSlotBack(Slot *slot);
	void addSlotBack(CBaseUIElement *element, bool scaleByHeightOnly=false);

	void insertSlot(Slot *slot, Slot *index);
	void insertSlot(CBaseUIElement *element, bool scaleByHeightOnly, Slot *index);
	void insertSlot(CBaseUIElement *element, Slot *index);
	void insertSlotBack(Slot *slot, Slot *index);
	void insertSlotBack(CBaseUIElement *element, Slot *index);
	void insertSlotBack(CBaseUIElement *element, bool scaleByHeightOnly, Slot *index);

	void removeSlot(Slot *slot);
	void deleteSlot(Slot *slot);

	Slot *getSlotByElementName(UString name);
	inline std::vector<Slot*> getAllSlots(){return m_vSlots;}
	inline std::vector<Slot*> *getAllSlotPointer(){return &m_vSlots;}
	std::vector<CBaseUIElement*> getAllElements();

	virtual void clear();
	virtual void empty();

	// setters
	virtual void setPos(float xPos, float yPos) {if (m_vPos.x != xPos || m_vPos.y != yPos) {m_vPos.x = xPos - m_vSize.x * m_vAnchor.x; m_vPos.y = yPos - m_vSize.y * m_vAnchor.y;}}
	virtual void setPosX(float xPos) {if (m_vPos.x != xPos) {m_vPos.x = xPos - m_vSize.x * m_vAnchor.x;}}
	virtual void setPosY(float yPos) {if (m_vPos.y != yPos) {m_vPos.y = yPos - m_vSize.y * m_vAnchor.y;}}
	virtual void setPos(Vector2 position) {if (m_vPos != position) {m_vPos = position - m_vSize * m_vAnchor;}}

	virtual void setPosAbsolute(float xPos, float yPos) {if (m_vPos.x != xPos || m_vPos.y != yPos) {m_vPos.x = xPos; m_vPos.y = yPos;}}
	virtual void setPosAbsoluteX(float xPos) {if (m_vPos.x != xPos) {m_vPos.x = xPos;}}
	virtual void setPosAbsoluteY(float yPos) {if (m_vPos.y != yPos) {m_vPos.y = yPos;}}
	virtual void setPosAbsolute(Vector2 position) {if (m_vPos != position) {m_vPos = position;}}

	virtual void setSize(float xSize, float ySize) {if (m_vSize.x != xSize || m_vSize.y != ySize) {m_vPos.x += (m_vSize.x - xSize) * m_vAnchor.x; m_vPos.y += (m_vSize.y - ySize) * m_vAnchor.y; m_vSize.x = xSize; m_vSize.y = ySize;}}
	virtual void setSizeX(float xSize) {if (m_vSize.x != xSize) {m_vPos.x += (m_vSize.x - xSize) * m_vAnchor.x; m_vSize.x = xSize;}}
	virtual void setSizeY(float ySize) {if (m_vSize.y != ySize) {m_vPos.y += (m_vSize.y - ySize) * m_vAnchor.y; m_vSize.y = ySize;}}
	virtual void setSize(Vector2 size) {if (m_vSize != size) {m_vPos += (m_vSize - size) * m_vAnchor; m_vSize = size;}}

	virtual void setSizeAbsolute(float xSize, float ySize) {if(m_vSize.x != xSize || m_vSize.y != ySize) {m_vSize.x = xSize; m_vSize.y = ySize;}}
	virtual void setSizeAbsoluteX(float xSize) {if(m_vSize.x != xSize) {m_vSize.x = xSize;}}
	virtual void setSizeAbsoluteY(float ySize) {if(m_vSize.y != ySize) {m_vSize.y = ySize;}}
	virtual void setSizeAbsolute(Vector2 size) {if(m_vSize != size) {m_vSize = size;}}

	virtual void setAnchor(float xAnchor, float yAnchor) {if (m_vAnchor.x != xAnchor || m_vAnchor.y != yAnchor){m_vPos.x -= m_vSize.x * (xAnchor - m_vAnchor.x); m_vPos.y -= m_vSize.y * (yAnchor - m_vAnchor.y); m_vAnchor.x = xAnchor; m_vAnchor.y = yAnchor;}}
	virtual void setAnchorX(float xAnchor){if (m_vAnchor.x != xAnchor){m_vPos.x -= m_vSize.x * (xAnchor - m_vAnchor.x); m_vAnchor.x = xAnchor;}}
	virtual void setAnchorY(float yAnchor){if (m_vAnchor.y != yAnchor){m_vPos.y -= m_vSize.y * (yAnchor - m_vAnchor.y); m_vAnchor.y = yAnchor;}}
	virtual void setAnchor(Vector2 anchor){if (m_vAnchor != anchor){m_vPos -= m_vSize * (anchor - m_vAnchor); m_vAnchor = anchor;}}

	virtual void setAnchorAbsolute(float xAnchor, float yAnchor) {if (m_vAnchor.x != xAnchor || m_vAnchor.y != yAnchor){m_vAnchor.x = xAnchor, m_vAnchor.y = yAnchor;}}
	virtual void setAnchorAbsoluteX(float xAnchor) {if (m_vAnchor.x != xAnchor) {m_vAnchor.x = xAnchor;}}
	virtual void setAnchorAbsoluteY(float yAnchor) {if (m_vAnchor.y != yAnchor) {m_vAnchor.y = yAnchor;}}
	virtual void setAnchorAbsolute(Vector2 anchor) {if (m_vAnchor != anchor) {m_vAnchor = anchor;}}

	virtual void setVisible(bool visible){m_bVisible = visible;}

	// getters
	inline const Vector2& getSize() const {return m_vSize;}

	virtual bool isVisible() {return m_bVisible;}

protected:
	// attributes
	Vector2 m_vSize;
	Vector2 m_vPos;
	Vector2 m_vAnchor;
	UString m_sName;
	bool m_bVisible;

protected:
	std::vector<Slot*> m_vSlots;
};

#endif /* GUI_WINDOWS_CBASEUICANVAS_H_ */
