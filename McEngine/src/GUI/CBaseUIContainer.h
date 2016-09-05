//================ Copyright (c) 2011, PG, All rights reserved. =================//
//
// Purpose:		a container for UI elements
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUICONTAINER_H
#define CBASEUICONTAINER_H

#include "CBaseUIElement.h"

class CBaseUIContainer : public CBaseUIElement
{
public:
	CBaseUIContainer(float xPos, float yPos, float xSize, float ySize, UString name);
	virtual ~CBaseUIContainer();
	void clear();
	void empty();

	void draw_debug(Graphics *g);
	virtual void draw(Graphics *g);
	virtual void update();

	void onKeyUp(KeyboardEvent &e);
	void onKeyDown(KeyboardEvent &e);
	void onChar(KeyboardEvent &e);

	void addBaseUIElement(CBaseUIElement *element, float xPos, float yPos);
	void addBaseUIElement(CBaseUIElement *element);
	void addBaseUIElementBack(CBaseUIElement *element, float xPos, float yPos);
	void addBaseUIElementBack(CBaseUIElement *element);

	void insertBaseUIElement(CBaseUIElement *element, CBaseUIElement *index);
	void insertBaseUIElementBack(CBaseUIElement *element, CBaseUIElement *index);

	void removeBaseUIElement(CBaseUIElement *element);
	void deleteBaseUIElement(CBaseUIElement *element);

	CBaseUIElement *getBaseUIElement(UString name);
	inline std::vector<CBaseUIElement*> getAllBaseUIElements(){return m_vElements;}
	inline std::vector<CBaseUIElement*> *getAllBaseUIElementsPointer(){return &m_vElements;}

	virtual void onMoved() {update_pos();}
	virtual void onResized() {update_pos();}

	virtual bool isBusy();
	virtual bool isActive();

	void onMouseDownOutside();

	virtual void onFocusStolen();
	virtual void onEnabled();
	virtual void onDisabled();

	void update_pos();
	void update_pos(CBaseUIElement *element);

protected:
	std::vector<CBaseUIElement*> m_vElements;
};

#endif
