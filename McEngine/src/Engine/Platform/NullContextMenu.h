//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		empty implementation of ContextMenu
//
// $NoKeywords: $ncmenu
//===============================================================================//

#ifndef NULLCONTEXTMENU_H
#define NULLCONTEXTMENU_H

#include "ContextMenu.h"

class NullContextMenu : public ContextMenu
{
public:
	NullContextMenu() {;}
	virtual ~NullContextMenu() {;}

	void begin() {;}
	void addItem(UString text, int returnValue) {;}
	void addSeparator() {;}
	int end() {return -1;}
};

#endif
