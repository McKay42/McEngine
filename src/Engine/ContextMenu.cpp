//================ Copyright (c) 2013, PG, All rights reserved. =================//
//
// Purpose:		top level context menu interface
//
// $NoKeywords: $cmenu
//===============================================================================//

// TODO: DEPRECATED

#include "ContextMenu.h"

ContextMenu *cmenu = NULL;

ContextMenu::ContextMenu()
{
	cmenu = this;
}

ContextMenu::~ContextMenu()
{
	cmenu = NULL;
}

