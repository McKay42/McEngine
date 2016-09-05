//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		top level context menu interface
//
// $NoKeywords: $cmenu
//===============================================================================//

// TODO: DEPRECATED

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include "cbase.h"

class ContextMenu
{
public:
	ContextMenu();
	virtual ~ContextMenu();

	static ContextMenu *get();

	virtual void begin() = 0;
	virtual void addItem(UString text, int returnValue) = 0;
	virtual void addSeparator() = 0;
	virtual int end() = 0;
};

extern ContextMenu *cmenu;

#endif
