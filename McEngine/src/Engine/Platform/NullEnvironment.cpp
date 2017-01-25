//================ Copyright (c) 2017, PG, All rights reserved. =================//
//
// Purpose:		empty environment, for debugging and new OS implementations
//
// $NoKeywords: $ne
//===============================================================================//

#include "NullEnvironment.h"
#include "Engine.h"

extern bool g_bRunning;

NullEnvironment::NullEnvironment()
{
}

void NullEnvironment::shutdown()
{
	g_bRunning = false;
}
