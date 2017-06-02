//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		a not so simple textbox, revision 4
//
// $NoKeywords: $
//===============================================================================//

#ifndef CBASEUITEXTBOX_H
#define CBASEUITEXTBOX_H

// TODO: rewrite this clusterfuck

#include "CBaseUIElement.h"

class McFont;

class CBaseUITextbox : public CBaseUIElement
{
public:
	CBaseUITextbox(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUITextbox(){;}

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onChar(KeyboardEvent &e);
	virtual void onKeyDown(KeyboardEvent &e);

	CBaseUITextbox *setDrawFrame(bool drawFrame){m_bDrawFrame = drawFrame; return this;}
	CBaseUITextbox *setDrawBackground(bool drawBackground){m_bDrawBackground = drawBackground; return this;}

	CBaseUITextbox *setBackgroundColor(Color backgroundColor){m_backgroundColor = backgroundColor; return this;}
	CBaseUITextbox *setTextColor(Color textColor){m_textColor = textColor; return this;}
	CBaseUITextbox *setCaretColor(Color caretColor){m_caretColor = caretColor; return this;}
	CBaseUITextbox *setFrameColor(Color frameColor){m_frameColor = frameColor; return this;}
	CBaseUITextbox *setFrameBrightColor(Color frameBrightColor) {m_frameBrightColor = frameBrightColor; return this;}
	CBaseUITextbox *setFrameDarkColor(Color frameDarkColor) {m_frameDarkColor = frameDarkColor; return this;}

	virtual CBaseUITextbox *setText(UString text);
	CBaseUITextbox *setFont(McFont *font);
	CBaseUITextbox *setTextAddX(float textAddX) {m_iTextAddX = textAddX; return this;}
	CBaseUITextbox *setCaretWidth(int caretWidth) {m_iCaretWidth = caretWidth; return this;}
	CBaseUITextbox *setTextJustification(int textJustification) {m_iTextJustification = textJustification;setText(m_sText); return this;}

	void setCursorPosRight();

	inline const UString getText() const {return m_sText;}
	inline McFont *getFont() const {return m_font;}

	bool hitEnter();
	bool hasSelectedText();
	void clear() {m_iSelectStart = m_iSelectEnd = 0;setText("");}

	// events
	virtual void onMouseDownInside();
	virtual void onMouseDownOutside();
	virtual void onMouseUpInside();
	virtual void onMouseUpOutside();
	virtual void onResized();

	// Overrides
	virtual CBaseUITextbox *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUITextbox *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUITextbox *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUITextbox *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUITextbox *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUITextbox *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUITextbox *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUITextbox *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUITextbox *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUITextbox *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUITextbox *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUITextbox *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUITextbox *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUITextbox *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUITextbox *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUITextbox *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUITextbox *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUITextbox *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUITextbox *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUITextbox *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUITextbox *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUITextbox *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUITextbox *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUITextbox *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUITextbox *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUITextbox *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUITextbox *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUITextbox *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUITextbox *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUITextbox *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUITextbox *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUITextbox *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUITextbox *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUITextbox *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUITextbox *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUITextbox *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUITextbox *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUITextbox *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUITextbox *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUITextbox *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUITextbox *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUITextbox *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUITextbox *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUITextbox *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUITextbox *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUITextbox *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUITextbox *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUITextbox *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUITextbox *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUITextbox *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


protected:
	virtual void drawText(Graphics *g);

	void tickCaret();
	void handleCaretKeyboardMove();
	void handleCaretKeyboardDelete();
	void updateCaretX();

	void handleDeleteSelectedText();
	void insertTextFromClipboard();
	void updateTextPos();
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
	int m_iSelectStart,m_iSelectEnd;
	int m_iSelectX;
};

#endif
