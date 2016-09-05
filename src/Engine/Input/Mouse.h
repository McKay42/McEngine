//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		mouse wrapper
//
// $NoKeywords: $mouse
//===============================================================================//

#ifndef MOUSE_H
#define MOUSE_H

#include "InputDevice.h"
#include "MouseListener.h"
#include "Cursors.h"

class Mouse : public InputDevice
{
public:
	Mouse();
	virtual ~Mouse() {;}

	virtual void draw(Graphics *g);
	void drawDebug(Graphics *g);
	virtual void update();

	void addListener(MouseListener *mouseListener, bool insertOnTop = false);
	void removeListener(MouseListener *mouseListener);

	void onPosChange(Vector2 pos);
	void onRawMove(int xDelta, int yDelta, bool absolute = false, bool virtualDesktop = false);

	void onWheelVertical(int delta);
	void onWheelHorizontal(int delta);

	void onLeftChange(bool leftDown);
	void onMiddleChange(bool middleDown);
	void onRightChange(bool rightDown);

	void setPos(Vector2 pos);
	void setCursorType(CURSORTYPE cursorType);
	void setCursorVisible(bool cursorVisible);

	void setOffset(Vector2 offset) {m_vOffset = offset;}
	void setScale(Vector2 scale) {m_vScale = scale;}

	inline Vector2 getPos() const {return m_vPos;}
	inline Vector2 getDelta() const {return m_vDelta;}
	inline Vector2 getRawDelta() const {return m_vRawDelta;}
	inline Vector2 getOffset() const {return m_vOffset;}

	inline bool isLeftDown() const {return m_bMouseLeftDown;}
	inline bool isMiddleDown() const {return m_bMouseMiddleDown;}
	inline bool isRightDown() const {return m_bMouseRightDown;}

	bool isCursorVisible();

	inline int getWheelDeltaVertical() const {return m_iWheelDeltaVertical;}
	inline int getWheelDeltaHorizontal() const {return m_iWheelDeltaHorizontal;}

private:
	Vector2 m_vPos;
	Vector2 m_vPrevPos;
	Vector2 m_vDelta;
	Vector2 m_vRawDelta;
	Vector2 m_vRawDeltaActual;

	bool m_bMouseLeftDown;
	bool m_bMouseMiddleDown;
	bool m_bMouseRightDown;

	int m_iWheelDeltaVertical;
	int m_iWheelDeltaHorizontal;
	int m_iWheelDeltaVerticalActual;
	int m_iWheelDeltaHorizontalActual;

	std::vector<MouseListener*> m_listeners;

	// custom
	bool m_bAbsolute;
	bool m_bVirtualDesktop;
	bool m_bSwitch;
	Vector2 m_vActualPos;
	Vector2 m_vOffset;
	Vector2 m_vScale;
	Rect desktopRect;
};

#endif
