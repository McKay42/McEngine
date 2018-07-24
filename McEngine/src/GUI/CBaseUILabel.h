//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		a simple label
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUILABEL_H
#define CBASEUILABEL_H

#include "CBaseUIElement.h"

class CBaseUILabel : public CBaseUIElement
{
public:
	enum TEXT_JUSTIFICATION
	{
		TEXT_JUSTIFICATION_LEFT,
		TEXT_JUSTIFICATION_CENTERED,
		TEXT_JUSTIFICATION_RIGHT
	};

public:
	CBaseUILabel(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="", UString text="");
	virtual ~CBaseUILabel() {;}

	ELEMENT_BODY(CBaseUILabel)

	virtual void draw(Graphics *g);
	virtual void update();

	// set
	CBaseUILabel *setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame; return this;}
	CBaseUILabel *setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground; return this;}

	CBaseUILabel *setFrameColor(Color frameColor) {m_frameColor = frameColor; return this;}
	CBaseUILabel *setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor; return this;}
	CBaseUILabel *setTextColor(Color textColor) {m_textColor = textColor; return this;}

	CBaseUILabel *setText(UString text) {m_sText = text; updateStringMetrics(); return this;}
	CBaseUILabel *setFont(McFont *font) {m_font = font; updateStringMetrics(); return this;}

	CBaseUILabel *setSizeToContent(int horizontalBorderSize = 1, int verticalBorderSize = 1) {setSize(m_fStringWidth+2*horizontalBorderSize, m_fStringHeight + 2*verticalBorderSize); return this;}
	CBaseUILabel *setWidthToContent(int horizontalBorderSize = 1) {setSizeX(m_fStringWidth+2*horizontalBorderSize); return this;}
	CBaseUILabel *setTextJustification(TEXT_JUSTIFICATION textJustification) {m_textJustification = textJustification; return this;}

	// DEPRECATED! use setTextJustification() instead
	void setCenterText(bool centerText) {m_bCenterText = centerText;}

	// get
	inline Color getFrameColor() const {return m_frameColor;}
	inline Color getBackgroundColor() const {return m_backgroundColor;}
	inline Color getTextColor() const {return m_textColor;}
	inline McFont *getFont() const {return m_font;}
	inline UString getText() const {return m_sText;}

	virtual void onResized() { updateStringMetrics(); }

protected:
	virtual void drawText(Graphics *g);

	void updateStringMetrics();

	McFont *m_font;
	UString m_sText;
	float m_fStringWidth;
	float m_fStringHeight;

	bool m_bDrawFrame;
	bool m_bDrawBackground;
	bool m_bCenterText;

	Color m_frameColor;
	Color m_backgroundColor;
	Color m_textColor;

	TEXT_JUSTIFICATION m_textJustification;
};

#endif
