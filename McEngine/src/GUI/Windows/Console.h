//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		console window, also handles ConVar parsing
//
// $NoKeywords: $con
//===============================================================================//

#ifndef CONSOLE_H
#define CONSOLE_H

#include "CBaseUIWindow.h"

class CBaseUITextbox;
class CBaseUIScrollView;
class CBaseUILabel;
class CBaseUITextField;
class McFont;

class Console : public CBaseUIWindow
{
public:
	static void processCommand(UString command);
	static void execConfigFile(UString filename);

public:
	Console();
	virtual ~Console();

	void update();

	void log(UString text, Color textColor = 0xffffffff);
	void clear();

	// events
	void onResized();

	static std::vector<UString> g_commandQueue;

private:
	CBaseUITextField *m_newLog;
	CBaseUIScrollView *m_log;
	CBaseUITextbox *m_textbox;

	McFont *m_logFont;
};

#endif
