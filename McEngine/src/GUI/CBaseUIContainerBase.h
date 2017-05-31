/*
 * CBaseUIContainerBase.h
 *
 *  Created on: May 31, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERBASE_H_
#define GUI_CBASEUICONTAINERBASE_H_

#include "CBaseUIElement.h"
#include "cbase.h"

class CBaseUIContainerBase : public CBaseUIElement
{
public:
	struct Slot
	{
		CBaseUIElement *element;
		bool scaleByHeightOnly = false;
	};

	CBaseUIContainerBase(UString name);
	virtual ~CBaseUIContainerBase();

	// container
	void addSlot(Slot *slot);
	void addSlot(CBaseUIElement *element, bool scaleByHeightOnly=false);
	void addSlotBack(Slot *slot);
	void addSlotBack(CBaseUIElement *element, bool scaleByHeightOnly=false);

	void insertSlot(Slot *index, Slot *slot);
	void insertSlot(Slot *index, CBaseUIElement *element, bool scaleByHeightOnly=false);
	void insertSlotBack(Slot *index, Slot *slot);
	void insertSlotBack(Slot *index, CBaseUIElement *element, bool scaleByHeightOnly=false);

	void removeSlot(Slot *slot);
	void deleteSlot(Slot *slot);

	Slot *getSlotByElementName(UString name);
	inline std::vector<Slot*> getAllSlots(){return m_vSlots;}
	inline std::vector<Slot*> *getAllSlotPointer(){return &m_vSlots;}
	std::vector<CBaseUIElement*> getAllElements();

	virtual void draw(Graphics *g);
	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0)) {;}
	virtual void update();

	virtual void clear();
	virtual void empty();

protected:
	// events
	virtual void updateSlot(Slot *slot) {;}

	std::vector<Slot *> m_vSlots;
};

#endif /* GUI_CBASEUICONTAINERBASE_H_ */
