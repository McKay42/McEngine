//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a not so simple textfield
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUITEXTFIELD_H
#define CBASEUITEXTFIELD_H

// TODO: finish this

#include "CBaseUIScrollView.h"

class CBaseUITextField : public CBaseUIScrollView
{
public:
	CBaseUITextField(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="", UString text="");
	virtual ~CBaseUITextField() {;}

	ELEMENT_BODY(CBaseUITextField)

	virtual void draw(Graphics *g);

	CBaseUITextField *setFont(McFont *font) {m_textObject->setFont(font); return this;}

	CBaseUITextField *append(UString text);

	void onResized();

protected:

	//********************************************************//
	//	The object which is added to this scrollview wrapper  //
	//********************************************************//

	class TextObject : public CBaseUIElement
	{
	public:
		TextObject(float xPos, float yPos, float width, float height, UString text);

		void draw(Graphics *g);

		CBaseUIElement *setText(UString text);
		CBaseUIElement *setFont(McFont *font) {m_font = font; updateStringMetrics(); return this;}

		CBaseUIElement *setTextColor(Color textColor) {m_textColor = textColor; return this;}
		CBaseUIElement *setParentSize(Vector2 parentSize) {m_vParentSize = parentSize;onResized(); return this;}

		inline Color getTextColor() const {return m_textColor;}

		inline UString getText() const {return m_sText;}
		inline McFont *getFont() const {return m_font;}

		void onResized();

	private:
		void updateStringMetrics();

		Vector2 m_vParentSize;

		UString m_sText;
		Color m_textColor;
		McFont *m_font;
		float m_fStringHeight;
	};

	TextObject *m_textObject;
};

#endif
