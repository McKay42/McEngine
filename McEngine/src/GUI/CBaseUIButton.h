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
	CBaseUIButton(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="", UString text="");
	virtual ~CBaseUIButton() {;}

	virtual void draw(Graphics *g);
	virtual void update();

	void click() {onClicked();}

	// callbacks, either void or with ourself as the argument
	typedef fastdelegate::FastDelegate0<> ButtonClickVoidCallback;
	CBaseUIButton *setClickCallback(ButtonClickVoidCallback clickCallback) {m_clickVoidCallback = clickCallback; return this;}
	typedef fastdelegate::FastDelegate1<CBaseUIButton*> ButtonClickCallback;
	CBaseUIButton *setClickCallback(ButtonClickCallback clickCallback) {m_clickCallback = clickCallback; return this;}

	// set
	CBaseUIButton *setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame; return this;}
	CBaseUIButton *setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground; return this;}
	CBaseUIButton *setTextLeft(bool textLeft) {m_bTextLeft = textLeft; updateStringMetrics(); return this;}

	CBaseUIButton *setFrameColor(Color frameColor) {m_frameColor = frameColor; return this;}
	CBaseUIButton *setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor; return this;}
	CBaseUIButton *setTextColor(Color textColor) {m_textColor = textColor;m_textBrightColor = m_textDarkColor = 0; return this;}
	CBaseUIButton *setTextBrightColor(Color textBrightColor) {m_textBrightColor = textBrightColor; return this;}
	CBaseUIButton *setTextDarkColor(Color textDarkColor) {m_textDarkColor = textDarkColor; return this;}

	CBaseUIButton *setText(UString text) {m_sText = text; updateStringMetrics(); return this;}
	CBaseUIButton *setFont(McFont *font) {m_font = font; updateStringMetrics(); return this;}

	CBaseUIButton *setSizeToContent(int horizontalBorderSize = 1, int verticalBorderSize = 1) {setSize(m_fStringWidth+2*horizontalBorderSize, m_fStringHeight + 2*verticalBorderSize); return this;}
	CBaseUIButton *setWidthToContent(int horizontalBorderSize = 1) {setSizeX(m_fStringWidth+2*horizontalBorderSize); return this;}

	// Overrides
	virtual CBaseUIButton *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUIButton *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUIButton *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUIButton *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUIButton *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUIButton *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUIButton *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUIButton *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUIButton *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUIButton *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUIButton *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUIButton *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUIButton *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUIButton *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUIButton *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUIButton *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUIButton *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUIButton *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUIButton *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUIButton *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUIButton *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIButton *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIButton *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIButton *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUIButton *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUIButton *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUIButton *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUIButton *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUIButton *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUIButton *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUIButton *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUIButton *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUIButton *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUIButton *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUIButton *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUIButton *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUIButton *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUIButton *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUIButton *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUIButton *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUIButton *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUIButton *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUIButton *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUIButton *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUIButton *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUIButton *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUIButton *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUIButton *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUIButton *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUIButton *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}

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
