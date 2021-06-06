//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a not so simple textbox, revision 4
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUITEXTBOX_H
#define CBASEUITEXTBOX_H

#include "CBaseUIElement.h"

class McFont;

class CBaseUITextbox : public CBaseUIElement
{
public:
	CBaseUITextbox(float xPos = 0.0f, float yPos = 0.0f, float xSize = 0.0f, float ySize = 0.0f, UString name = "");
	virtual ~CBaseUITextbox() {;}

	ELEMENT_BODY(CBaseUITextbox)

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onChar(KeyboardEvent &e);
	virtual void onKeyDown(KeyboardEvent &e);

	inline const UString getText() const {return m_sText;}
	inline UString &getTextRef() {return m_sText;} // DEPRECATED
	inline McFont *getFont() const {return m_font;}

	CBaseUITextbox *setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame; return this;}
	CBaseUITextbox *setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground; return this;}

	CBaseUITextbox *setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor; return this;}
	CBaseUITextbox *setTextColor(Color textColor) {m_textColor = textColor; return this;}
	CBaseUITextbox *setCaretColor(Color caretColor) {m_caretColor = caretColor; return this;}
	CBaseUITextbox *setFrameColor(Color frameColor) {m_frameColor = frameColor; return this;}
	CBaseUITextbox *setFrameBrightColor(Color frameBrightColor) {m_frameBrightColor = frameBrightColor; return this;}
	CBaseUITextbox *setFrameDarkColor(Color frameDarkColor) {m_frameDarkColor = frameDarkColor; return this;}

	CBaseUITextbox *setFont(McFont *font);
	CBaseUITextbox *setTextAddX(float textAddX) {m_iTextAddX = textAddX; return this;}
	CBaseUITextbox *setCaretWidth(int caretWidth) {m_iCaretWidth = caretWidth; return this;}
	CBaseUITextbox *setTextJustification(int textJustification) {m_iTextJustification = textJustification; setText(m_sText); return this;}

	virtual CBaseUITextbox *setText(UString text);

	void setCursorPosRight();

	bool hitEnter();
	bool hasSelectedText() const;
	void clear();

protected:
	virtual void drawText(Graphics *g);

	// events
	virtual void onMouseDownInside();
	virtual void onMouseDownOutside();
	virtual void onMouseUpInside();
	virtual void onMouseUpOutside();
	virtual void onResized();

	void tickCaret();
	void handleCaretKeyboardMove();
	void handleCaretKeyboardDelete();
	void updateCaretX();

	void handleDeleteSelectedText();
	void insertTextFromClipboard();
	void updateTextPos();
	void deselectText();
	UString getSelectedText();

	UString m_sText;

	McFont *m_font;

	Color m_textColor;
	Color m_frameColor;
	Color m_frameBrightColor;
	Color m_frameDarkColor;
	Color m_caretColor;
	Color m_backgroundColor;

	bool m_bContextMouse;
	bool m_bBlockMouse;
	bool m_bCatchMouse;
	bool m_bDrawFrame;
	bool m_bDrawBackground;
	bool m_bLine;

	int m_iTextAddX;
	int m_iTextAddY;
	float m_fTextScrollAddX;
	int m_iCaretPosition;
	int m_iCaretX;
	int m_iCaretWidth;
	int m_iTextJustification;

	float m_fLinetime;
	float m_fTextWidth;

	bool m_bHitenter;

	bool m_bSelectCheck;
	int m_iSelectStart;
	int m_iSelectEnd;
	int m_iSelectX;
};

#endif
