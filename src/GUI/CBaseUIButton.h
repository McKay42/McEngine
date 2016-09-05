//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		a simple button
//
// $NoKeywords: $button
//===============================================================================//

#ifndef CBASEUIBUTTON_H
#define CBASEUIBUTTON_H

#include "CBaseUIElement.h"

class McFont;

class CBaseUIButton : public CBaseUIElement
{
public:
	CBaseUIButton(float xPos, float yPos, float xSize, float ySize, UString name, UString text);
	virtual ~CBaseUIButton() {;}

	virtual void draw(Graphics *g);
	virtual void update();

	void click() {onClicked();}

	// callbacks, either void or with ourself as the argument
	typedef fastdelegate::FastDelegate0<> ButtonClickVoidCallback;
	void setClickCallback(ButtonClickVoidCallback clickCallback) {m_clickVoidCallback = clickCallback;}
	typedef fastdelegate::FastDelegate1<CBaseUIButton*> ButtonClickCallback;
	void setClickCallback(ButtonClickCallback clickCallback) {m_clickCallback = clickCallback;}

	// set
	void setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame;}
	void setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground;}
	void setTextLeft(bool textLeft) {m_bTextLeft = textLeft; updateStringMetrics();}

	void setFrameColor(Color frameColor) {m_frameColor = frameColor;}
	void setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor;}
	void setTextColor(Color textColor) {m_textColor = textColor;m_textBrightColor = m_textDarkColor = 0;}
	void setTextBrightColor(Color textBrightColor) {m_textBrightColor = textBrightColor;}
	void setTextDarkColor(Color textDarkColor) {m_textDarkColor = textDarkColor;}

	void setText(UString text) {m_sText = text; updateStringMetrics();}
	void setFont(McFont *font) {m_font = font; updateStringMetrics();}

	void setSizeToContent(int horizontalBorderSize = 1, int verticalBorderSize = 1) {setSize(m_fStringWidth+2*horizontalBorderSize, m_fStringHeight + 2*verticalBorderSize);}
	void setWidthToContent(int horizontalBorderSize = 1) {setSizeX(m_fStringWidth+2*horizontalBorderSize);}

	// get
	inline Color getFrameColor() const {return m_frameColor;}
	inline Color getBackgroundColor() const {return m_backgroundColor;}
	inline Color getTextColor() const {return m_textColor;}
	inline UString getText() const {return m_sText;}
	inline McFont *getFont() const {return m_font;}
	inline ButtonClickCallback getClickCallback() const {return m_clickCallback;}
	inline bool isTextLeft() const {return m_bTextLeft;}

	// events
	virtual void onMouseUpInside();
	virtual void onResized() {updateStringMetrics();}

protected:
	virtual void onClicked();

	void drawText(Graphics *g);
	void drawHoverRect(Graphics *g, int distance);

	void updateStringMetrics();

	bool m_bDrawFrame;
	bool m_bDrawBackground;
	bool m_bTextLeft;

	Color m_frameColor;
	Color m_backgroundColor;
	Color m_textColor;
	Color m_textBrightColor;
	Color m_textDarkColor;

	McFont *m_font;
	UString m_sText;
	float m_fStringWidth;
	float m_fStringHeight;

	ButtonClickVoidCallback m_clickVoidCallback;
	ButtonClickCallback m_clickCallback;
};

#endif
