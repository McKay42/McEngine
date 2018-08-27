//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		app base class (v2)
//
// $NoKeywords: $appb
//===============================================================================//

#ifndef APP_H
#define APP_H

#include "cbase.h"
#include "KeyboardListener.h"

class Engine;

class App : public KeyboardListener
{
public:
	App() {;}
	virtual ~App() {;}

	virtual void draw(Graphics *g) {;}
	virtual void update() {;}

	virtual void onKeyDown(KeyboardEvent &e) {;}
	virtual void onKeyUp(KeyboardEvent &e) {;}
	virtual void onChar(KeyboardEvent &e) {;}

	virtual void onResolutionChanged(Vector2 newResolution) {;}

	virtual void onFocusGained() {;}
	virtual void onFocusLost() {;}

	virtual void onMinimized() {;}
	virtual void onRestored() {;}

	virtual bool onShutdown() {return true;}
};

#endif
