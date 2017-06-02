//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		a simple slider
//
// $NoKeywords: $
//===============================================================================//

// TODO: fix vertical sliders
// TODO: this entire class is a mess

#ifndef CBASEUISLIDER_H
#define CBASEUISLIDER_H

#include "CBaseUIElement.h"

class CBaseUISlider : public CBaseUIElement
{
public:
	CBaseUISlider(float xPos=0, float yPos=0, float xSize=0, float ySize=0, UString name="");
	virtual ~CBaseUISlider(){;}

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onKeyDown(KeyboardEvent &e);

	void forceCallCallback();

	void setOrientation(bool horizontal) {m_bHorizontal = horizontal;onResized();}

	void setDrawFrame(bool drawFrame) {m_bDrawFrame = drawFrame;}
	void setDrawBackground(bool drawBackground) {m_bDrawBackground = drawBackground;}

	void setFrameColor(Color frameColor) {m_frameColor = frameColor;}
	void setBackgroundColor(Color backgroundColor) {m_backgroundColor = backgroundColor;}

	void setBlockSize(float xSize, float ySize);

	// callbacks, either void or with ourself as the argument
	typedef fastdelegate::FastDelegate1<CBaseUISlider*> SliderChangeCallback;
	CBaseUISlider *setChangeCallback(SliderChangeCallback changeCallback) {m_sliderChangeCallback = changeCallback; return this;}

	CBaseUISlider *setAllowMouseWheel(bool allowMouseWheel) {m_bAllowMouseWheel = allowMouseWheel; return this;}
	CBaseUISlider *setAnimated(bool animated) {m_bAnimated = animated; return this;}
	CBaseUISlider *setLiveUpdate(bool liveUpdate) {m_bLiveUpdate = liveUpdate; return this;}
	CBaseUISlider *setBounds(float minValue, float maxValue);
	CBaseUISlider *setKeyDelta(float keyDelta) {m_fKeyDelta = keyDelta; return this;}
	CBaseUISlider *setValue(float value, bool animate = true);
	CBaseUISlider *setInitialValue(float value);

	inline float getFloat() {return m_fCurValue;}
	inline int getInt() {return (int)m_fCurValue;}
	inline bool getBool() {return (bool)m_fCurValue;}
	inline float getMax() {return m_fMaxValue;}
	inline float getMin() {return m_fMinValue;}
	float getPercent();

	// TODO: DEPRECATED, don't use this function anymore, use setChangeCallback() instead
	bool hasChanged();

	virtual void onFocusStolen();
	virtual void onMouseUpInside();
	virtual void onMouseUpOutside();
	virtual void onMouseDownInside();
	virtual void onResized();

	// Overrides
	virtual CBaseUISlider *setPos(float posX, float posY) {CBaseUIElement::setPos(posX, posY); return this;}
	virtual CBaseUISlider *setPosX(float posX) {CBaseUIElement::setPosX(posX); return this;}
	virtual CBaseUISlider *setPosY(float posY) {CBaseUIElement::setPosY(posY); return this;}
	virtual CBaseUISlider *setPos(Vector2 pos) {CBaseUIElement::setPos(pos); return this;}

	virtual CBaseUISlider *setPosAbsolute(float posX, float posY) {CBaseUIElement::setPosAbsolute(posX, posY); return this;}
	virtual CBaseUISlider *setPosAbsoluteX(float posX) {CBaseUIElement::setPosAbsoluteX(posX); return this;}
	virtual CBaseUISlider *setPosAbsoluteY(float posY) {CBaseUIElement::setPosAbsoluteY(posY); return this;}
	virtual CBaseUISlider *setPosAbsolute(Vector2 pos) {CBaseUIElement::setPosAbsolute(pos); return this;}

	virtual CBaseUISlider *setRelPos(float posX, float posY) {CBaseUIElement::setRelPos(posX, posY); return this;}
	virtual CBaseUISlider *setRelPosX(float posX) {CBaseUIElement::setRelPosX(posX); return this;}
	virtual CBaseUISlider *setRelPosY(float posY) {CBaseUIElement::setRelPosY(posY); return this;}
	virtual CBaseUISlider *setRelPos(Vector2 pos) {CBaseUIElement::setRelPos(pos); return this;}

	virtual CBaseUISlider *setRelPosAbsolute(float posX, float posY) {CBaseUIElement::setRelPosAbsolute(posX, posY); return this;}
	virtual CBaseUISlider *setRelPosAbsoluteX(float posX) {CBaseUIElement::setRelPosAbsoluteX(posX); return this;}
	virtual CBaseUISlider *setRelPosAbsoluteY(float posY) {CBaseUIElement::setRelPosAbsoluteY(posY); return this;}
	virtual CBaseUISlider *setRelPosAbsolute(Vector2 pos) {CBaseUIElement::setRelPosAbsolute(pos); return this;}

	virtual CBaseUISlider *setSize(float sizeX, float sizeY) {CBaseUIElement::setSize(sizeX, sizeY); return this;}
	virtual CBaseUISlider *setSizeX(float sizeX) {CBaseUIElement::setSizeX(sizeX); return this;}
	virtual CBaseUISlider *setSizeY(float sizeY) {CBaseUIElement::setSizeY(sizeY); return this;}
	virtual CBaseUISlider *setSize(Vector2 size) {CBaseUIElement::setSize(size); return this;}

	virtual CBaseUISlider *setSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUISlider *setSizeAbsoluteX(float sizeX) {CBaseUIElement::setSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUISlider *setSizeAbsoluteY(float sizeY) {CBaseUIElement::setSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUISlider *setSizeAbsolute(Vector2 size) {CBaseUIElement::setSizeAbsolute(size); return this;}

	virtual CBaseUISlider *setRelSize(float sizeX, float sizeY) {CBaseUIElement::setRelSize(sizeX, sizeY); return this;}
	virtual CBaseUISlider *setRelSizeX(float sizeX) {CBaseUIElement::setRelSizeX(sizeX); return this;}
	virtual CBaseUISlider *setRelSizeY(float sizeY) {CBaseUIElement::setRelSizeY(sizeY); return this;}
	virtual CBaseUISlider *setRelSize(Vector2 size) {CBaseUIElement::setRelSize(size); return this;}

	virtual CBaseUISlider *setRelSizeAbsolute(float sizeX, float sizeY) {CBaseUIElement::setRelSizeAbsolute(sizeX, sizeY); return this;}
	virtual CBaseUISlider *setRelSizeAbsoluteX(float sizeX) {CBaseUIElement::setRelSizeAbsoluteX(sizeX); return this;}
	virtual CBaseUISlider *setRelSizeAbsoluteY(float sizeY) {CBaseUIElement::setRelSizeAbsoluteY(sizeY); return this;}
	virtual CBaseUISlider *setRelSizeAbsolute(Vector2 size) {CBaseUIElement::setRelSizeAbsolute(size); return this;}

	virtual CBaseUISlider *setAnchor(float anchorX, float anchorY) {CBaseUIElement::setAnchor(anchorX, anchorY); return this;}
	virtual CBaseUISlider *setAnchorX(float anchorX) {CBaseUIElement::setAnchorX(anchorX); return this;}
	virtual CBaseUISlider *setAnchorY(float anchorY) {CBaseUIElement::setAnchorY(anchorY); return this;}
	virtual CBaseUISlider *setAnchor(Vector2 anchor) {CBaseUIElement::setAnchor(anchor); return this;}

	virtual CBaseUISlider *setAnchorAbsolute(float anchorX, float anchorY) {CBaseUIElement::setAnchorAbsolute(anchorX, anchorY); return this;}
	virtual CBaseUISlider *setAnchorAbsoluteX(float anchorX) {CBaseUIElement::setAnchorAbsoluteX(anchorX); return this;}
	virtual CBaseUISlider *setAnchorAbsoluteY(float anchorY) {CBaseUIElement::setAnchorAbsoluteY(anchorY); return this;}
	virtual CBaseUISlider *setAnchorAbsolute(Vector2 anchor) {CBaseUIElement::setAnchorAbsolute(anchor); return this;}

	virtual CBaseUISlider *setVisible(bool visible) {CBaseUIElement::setVisible(visible); return this;}
	virtual CBaseUISlider *setActive(bool active) {CBaseUIElement::setActive(active); return this;}
	virtual CBaseUISlider *setKeepActive(bool keepActive) {CBaseUIElement::setKeepActive(keepActive); return this;}
	virtual CBaseUISlider *setDrawManually(bool drawManually) {CBaseUIElement::setDrawManually(drawManually); return this;}
	virtual CBaseUISlider *setPositionManually(bool positionManually) {CBaseUIElement::setPositionManually(positionManually); return this;}
	virtual CBaseUISlider *setEnabled(bool enabled) {CBaseUIElement::setEnabled(enabled); return this;}
	virtual CBaseUISlider *setBusy(bool busy) {CBaseUIElement::setBusy(busy); return this;}
	virtual CBaseUISlider *setName(UString name) {CBaseUIElement::setName(name); return this;}
	virtual CBaseUISlider *setParent(CBaseUIElement *parent) {CBaseUIElement::setParent(parent); return this;}
	virtual CBaseUISlider *setScaleByHeightOnly(bool scaleByHeightOnly) {CBaseUIElement::setScaleByHeightOnly(scaleByHeightOnly); return this;}


protected:
	virtual void drawBlock(Graphics *g);

	void updateBlockPos();

	bool m_bDrawFrame,m_bDrawBackground;
	bool m_bHorizontal;
	bool m_bHasChanged;
	bool m_bAnimated;
	bool m_bLiveUpdate;
	bool m_bAllowMouseWheel;
	Color m_frameColor,m_backgroundColor;

	float m_fMinValue,m_fMaxValue,m_fCurValue,m_fCurPercent;
	Vector2 m_vBlockSize,m_vBlockPos;

	Vector2 m_vGrabBackup;
	float m_fPrevValue;

	float m_fKeyDelta;

	SliderChangeCallback m_sliderChangeCallback;
};

#endif
