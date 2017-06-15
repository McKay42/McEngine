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

	ELEMENT_BODY(CBaseUISlider)

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
