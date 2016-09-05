//================ Copyright (c) 2012, PG, All rights reserved. =================//
//
// Purpose:		windows context menu interface
//
// $NoKeywords: $wincmenu
//===============================================================================//

// TODO: DEPRECATED

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinContextMenu.h"
#include "WinEnvironment.h"
#include "Engine.h"

WinContextMenu::WinContextMenu()
{
	m_menu = NULL;
}

WinContextMenu::~WinContextMenu()
{
}

void WinContextMenu::begin()
{
	m_menu = CreatePopupMenu();
}

void WinContextMenu::addItem(UString text, int returnValue)
{
	InsertMenu(m_menu, 0, MF_BYPOSITION | MF_STRING, returnValue, text.toUtf8());
}

void WinContextMenu::addSeparator()
{
	MENUITEMINFO mySep;

	mySep.cbSize = sizeof(MENUITEMINFO);
	mySep.fMask = MIIM_TYPE;
	mySep.fType = MFT_SEPARATOR;

	InsertMenuItem(m_menu, 0, 1, &mySep);
}

int WinContextMenu::end()
{
    engine->focus();

	POINT p;
	GetCursorPos(&p);

	return TrackPopupMenu(m_menu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD, p.x, p.y, 0, ((WinEnvironment*)env)->getHwnd(), NULL);
}

#endif
