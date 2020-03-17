//================ Copyright (c) 2020, PG, All rights reserved. =================//
//
// Purpose:		horizon implementation of thread
//
// $NoKeywords: $nxthread $os
//===============================================================================//

#ifdef __SWITCH__

#include "HorizonThread.h"

#include "Engine.h"

HorizonThread::HorizonThread(McThread::START_ROUTINE start_routine, void *arg) : BaseThread()
{
	Result rc = threadCreate((Thread*)&m_thread, (ThreadFunc)start_routine, arg, NULL, 0x1000000, 0x2B, 2);

	m_bReady = !R_FAILED(rc);

	if (!m_bReady)
		debugLog("HorizonThread Error: threadCreate() returned %i!\n", (int)rc);
	else
		threadStart((Thread*)&m_thread);
}

HorizonThread::~HorizonThread()
{
	if (!m_bReady) return;

	m_bReady = false;

	threadWaitForExit((Thread*)&m_thread);
	threadClose((Thread*)&m_thread);
}

#endif
