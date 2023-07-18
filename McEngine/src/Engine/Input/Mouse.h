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

	void resetWheelDelta();

	void onPosChange(Vector2 pos);
	void onRawMove(int xDelta, int yDelta, bool absolute = false, bool virtualDesktop = false);

	void onWheelVertical(int delta);
	void onWheelHorizontal(int delta);

	void onLeftChange(bool leftDown);
	void onMiddleChange(bool middleDown);
	void onRightChange(bool rightDown);
	void onButton4Change(bool button4down);
	void onButton5Change(bool button5down);

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
	inline bool isButton4Down() const {return m_bMouse4Down;}
	inline bool isButton5Down() const {return m_bMouse5Down;}

	bool isCursorVisible();

	inline int getWheelDeltaVertical() const {return m_iWheelDeltaVertical;}
	inline int getWheelDeltaHorizontal() const {return m_iWheelDeltaHorizontal;}

private:
	void setPosXY(float x, float y); // shit hack

	Vector2 m_vPos;
	Vector2 m_vPosWithoutOffset;
	Vector2 m_vPrevOsMousePos;
	Vector2 m_vDelta;

	Vector2 m_vRawDelta;
	Vector2 m_vRawDeltaActual;
	Vector2 m_vRawDeltaAbsolute;
	Vector2 m_vRawDeltaAbsoluteActual;

	bool m_bMouseLeftDown;
	bool m_bMouseMiddleDown;
	bool m_bMouseRightDown;
	bool m_bMouse4Down;
	bool m_bMouse5Down;

	int m_iWheelDeltaVertical;
	int m_iWheelDeltaHorizontal;
	int m_iWheelDeltaVerticalActual;
	int m_iWheelDeltaHorizontalActual;

	std::vector<MouseListener*> m_listeners;

	// custom
	bool m_bSetPosWasCalledLastFrame;
	bool m_bAbsolute;
	bool m_bVirtualDesktop;
	Vector2 m_vActualPos;
	Vector2 m_vOffset;
	Vector2 m_vScale;
	McRect desktopRect;

	struct FAKELAG_PACKET
	{
		float time;
		Vector2 pos;
	};
	std::vector<FAKELAG_PACKET> m_fakelagBuffer;
	Vector2 m_vFakeLagPos;
};

#endif
