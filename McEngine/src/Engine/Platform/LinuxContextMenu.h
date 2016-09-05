//================ Copyright (c) 2014, PG, All rights reserved. =================//
//
// Purpose:		linux context menu interface
//
// $NoKeywords: $linuxcmenu
//===============================================================================//

// TODO: DEPRECATED

#ifdef __linux__

#ifndef LINUXCONTEXTMENU_H
#define LINUXCONTEXTMENU_H

#include "ContextMenu.h"

class LinuxContextMenu : public ContextMenu
{
public:
	LinuxContextMenu();
	virtual ~LinuxContextMenu();

	void begin() {;}
	void addItem(UString text, int returnValue) {;}
	void addSeparator() {;}
	int end() {return -1;}

private:
};

#endif

#endif
