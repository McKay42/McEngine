//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		simple listener class for mouse events
//
// $NoKeywords: $mouse
//===============================================================================//

#ifndef MOUSELISTENER_H
#define MOUSELISTENER_H

class MouseListener
{
public:
	virtual ~MouseListener() {;}

	virtual void onLeftChange(bool down) = 0;
	virtual void onMiddleChange(bool down) = 0;
	virtual void onRightChange(bool down) = 0;

	virtual void onWheelVertical(int delta) = 0;
	virtual void onWheelHorizontal(int delta) = 0;
};

#endif
