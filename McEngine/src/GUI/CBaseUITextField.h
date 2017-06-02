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

	virtual void draw(Graphics *g);

	CBaseUITextField *setFont(McFont *font) {m_textObject->setFont(font); return this;}

	CBaseUITextField *append(UString text);

	void onResized();

	// Overrides
	virtual CBaseUITextField *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUITextField *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUITextField *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUITextField *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUITextField *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUITextField *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUITextField *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUITextField *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUITextField *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUITextField *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUITextField *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUITextField *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUITextField *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUITextField *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUITextField *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUITextField *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUITextField *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUITextField *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUITextField *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUITextField *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUITextField *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUITextField *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUITextField *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUITextField *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUITextField *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUITextField *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUITextField *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUITextField *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUITextField *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUITextField *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUITextField *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUITextField *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUITextField *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUITextField *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUITextField *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUITextField *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUITextField *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUITextField *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUITextField *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUITextField *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUITextField *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUITextField *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUITextField *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUITextField *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUITextField *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUITextField *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUITextField *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUITextField *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUITextField *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUITextField *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


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
