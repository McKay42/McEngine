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

	// Overrides
	virtual CBaseUILabel *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUILabel *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUILabel *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUILabel *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUILabel *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUILabel *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUILabel *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUILabel *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUILabel *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUILabel *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUILabel *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUILabel *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUILabel *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUILabel *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUILabel *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUILabel *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUILabel *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUILabel *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUILabel *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUILabel *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUILabel *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUILabel *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUILabel *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUILabel *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUILabel *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUILabel *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUILabel *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUILabel *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUILabel *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUILabel *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUILabel *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUILabel *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUILabel *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUILabel *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUILabel *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUILabel *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUILabel *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUILabel *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUILabel *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUILabel *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUILabel *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUILabel *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUILabel *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUILabel *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUILabel *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUILabel *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUILabel *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUILabel *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUILabel *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUILabel *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


protected:
	void updateStringMetrics();

	void drawText(Graphics *g);

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
