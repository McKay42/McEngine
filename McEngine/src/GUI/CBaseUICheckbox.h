//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple checkbox
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUICHECKBOX_H
#define CBASEUICHECKBOX_H

#include "CBaseUIButton.h"

class CBaseUICheckbox : public CBaseUIButton
{
public:
	CBaseUICheckbox(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="", UString text="");
	virtual ~CBaseUICheckbox() {;}

	ELEMENT_BODY(CBaseUICheckbox)

	virtual void draw(Graphics *g);

	inline float getBlockSize() {return m_vSize.y/2;}
	inline float getBlockBorder() {return m_vSize.y/4;}
	inline bool isChecked() const {return m_bChecked;}

	CBaseUICheckbox *setChecked(bool checked, bool fireChangeEvent = true);
	CBaseUICheckbox *setSizeToContent(int horizontalBorderSize = 1, int verticalBorderSize = 1);
	CBaseUICheckbox *setWidthToContent(int horizontalBorderSize = 1);

	typedef fastdelegate::FastDelegate1<CBaseUICheckbox*> CheckboxChangeCallback;
	CBaseUICheckbox *setChangeCallback( CheckboxChangeCallback clickCallback ) {m_changeCallback = clickCallback; return this;}

protected:
	void onPressed();

	bool m_bChecked;
	CheckboxChangeCallback m_changeCallback;
};


#endif
