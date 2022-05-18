//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		enables a single CBaseUIWindow to behave like a native window
//
// $NoKeywords: $
//===============================================================================//

#ifndef GUICOHERENCEMODE_H
#define GUICOHERENCEMODE_H

#include "App.h"

class CBaseUIWindow;

class GUICoherenceMode : public App
{
public:
	GUICoherenceMode();
	virtual ~GUICoherenceMode();

	virtual void draw(Graphics *g);
	virtual void update();

	virtual void onKeyDown(KeyboardEvent &key);
	virtual void onKeyUp(KeyboardEvent &key);
	virtual void onChar(KeyboardEvent &charCode);

	virtual void onResolutionChanged(Vector2 newResolution);

private:
	CBaseUIWindow *m_window;
};

#endif
