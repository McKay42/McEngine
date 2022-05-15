//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		enables a single CBaseUIWindow to behave like a native window
//
// $NoKeywords: $
//===============================================================================//

#include "GUICoherenceMode.h"

#include "Engine.h"
#include "ConVar.h"

#include "Console.h"
#include "VinylScratcher.h"

GUICoherenceMode::GUICoherenceMode()
{
	// NOTE: uncomment WINDOW_FRAMELESS in main_Windows.cpp to let the engine control everything (including window borders and titlebars etc.)

	m_window = new VinylScratcher();
	//m_window = new Console();
	///engine->setConsole((Console*)m_window);

	m_window->enableCoherenceMode();
	m_window->open();

	// engine overrides
	convar->getConVarByName("console_overlay")->setValue(0.0f);
	convar->getConVarByName("fps_max")->setValue(420.0f);
	convar->getConVarByName("fps_max_background")->setValue(10.0f);
	convar->getConVarByName("fps_max_background_interleaved")->setValue(10.0f);
	convar->getConVarByName("vsync")->setValue(1.0f);

	Console::execConfigFile("autoexec.cfg");
}

GUICoherenceMode::~GUICoherenceMode()
{
	///engine->setConsole(NULL);

	SAFE_DELETE(m_window);
}

void GUICoherenceMode::draw(Graphics *g)
{
	m_window->draw(g);
}

void GUICoherenceMode::update()
{
	m_window->update();
}

void GUICoherenceMode::onKeyDown(KeyboardEvent &key)
{
	m_window->onKeyDown(key);
}

void GUICoherenceMode::onKeyUp(KeyboardEvent &key)
{
	m_window->onKeyUp(key);
}

void GUICoherenceMode::onChar(KeyboardEvent &charCode)
{
	m_window->onChar(charCode);
}

void GUICoherenceMode::onResolutionChanged(Vector2 newResolution)
{
	m_window->onResolutionChange(newResolution);
}
