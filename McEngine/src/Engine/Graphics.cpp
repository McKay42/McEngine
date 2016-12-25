//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		top level graphics interface
//
// $NoKeywords: $graphics
//===============================================================================//

#include "Graphics.h"

#include "Engine.h"
#include "ConVar.h"

ConVar _r_globaloffset_x("r_globaloffset_x", 0.0f);
ConVar _r_globaloffset_y("r_globaloffset_y", 0.0f);
ConVar _r_debug_disable_cliprect("r_debug_disable_cliprect", false);
ConVar _r_debug_disable_3dscene("r_debug_disable_3dscene", false);
ConVar _r_debug_flush_drawstring("r_debug_flush_drawstring", false);
ConVar _r_debug_drawimage("r_debug_drawimage", false);

ConVar *Graphics::r_globaloffset_x = &_r_globaloffset_x;
ConVar *Graphics::r_globaloffset_y = &_r_globaloffset_y;
ConVar *Graphics::r_debug_disable_cliprect = &_r_debug_disable_cliprect;
ConVar *Graphics::r_debug_disable_3dscene = &_r_debug_disable_3dscene;
ConVar *Graphics::r_debug_flush_drawstring = &_r_debug_flush_drawstring;
ConVar *Graphics::r_debug_drawimage = &_r_debug_drawimage;

void _vsync(UString oldValue, UString newValue)
{
	if (newValue.length() < 1)
		debugLog("Usage: 'vsync 1' to turn vsync on, 'vsync 0' to turn vsync off\n");
	else
	{
		bool vsync = newValue.toFloat() > 0.0f;
		engine->getGraphics()->setVSync(vsync);
	}
}

void _mat_wireframe(UString oldValue, UString newValue)
{
	engine->getGraphics()->setWireframe(newValue.toFloat() > 0.0f);
}

ConVar __mat_wireframe("mat_wireframe", false, _mat_wireframe);
ConVar __vsync("vsync", false, _vsync);



