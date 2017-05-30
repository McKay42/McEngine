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
#include "CBaseUIElement.h"

/*
 * UI Canvas Container
 * Scales any slotted containers or elements by the size of the canvas, useful for resolution scaling
 * The size/position of UI elements slotted should 0.0 to 1.0 as a percentage of the total screen area
 * Set scaleByHeightOnly per element to avoid stretching/squashing on aspect ratio changes. Uses a 16:9 (Widescreen) aspect ratio for assumed desired width
 */

class CBaseUICanvas : public CBaseUIElement
{
public:
	struct Slot{
		CBaseUIElement *element;
		bool scaleByHeightOnly=false;
	};

	CBaseUICanvas(float xPos, float yPos, float xSize, float ySize, UString name);
	virtual ~CBaseUICanvas();

	// main
	virtual void draw(Graphics *g);
	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0));
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

protected:
	// events
	virtual void onMoved();
	virtual void onResized();
	virtual void updateLayout();

	std::vector<Slot*> m_vSlots;
};

#endif /* GUI_WINDOWS_CBASEUICANVAS_H_ */
