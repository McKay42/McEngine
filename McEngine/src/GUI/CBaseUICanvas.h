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
 * Set scaleByHeightOnly per element to avoid stretching/squashing on aspect ratio changes
 */
struct Slot{
	CBaseUIElement *element;
	bool scaleByHeightOnly=false;
};

class CBaseUICanvas {
public:
	CBaseUICanvas(float xSize, float ySize);
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
	virtual void setSize(float xSize, float ySize){if(m_vSize.x != xSize || m_vSize.y != ySize){m_vSize.x = xSize; m_vSize.y = ySize;}}
	virtual void setSizeX(float xSize){if(m_vSize.x != xSize){m_vSize.x = xSize;}}
	virtual void setSizeY(float ySize){if(m_vSize.y != ySize){m_vSize.y = ySize;}}
	virtual void setSize(Vector2 size){if(m_vSize != size){m_vSize = size;}}

	virtual void setVisible(bool visible){m_bVisible = visible;}

	// getters
	inline const Vector2& getSize() const {return m_vSize;}

	virtual bool isVisible() {return m_bVisible;}

protected:
	// attributes
	Vector2 m_vSize;
	UString m_sName;
	bool m_bVisible;

protected:
	std::vector<Slot*> m_vSlots;
};

#endif /* GUI_WINDOWS_CBASEUICANVAS_H_ */
