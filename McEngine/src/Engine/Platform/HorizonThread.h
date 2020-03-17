//================ Copyright (c) 2020, PG, All rights reserved. =================//
//
// Purpose:		horizon implementation of thread
//
// $NoKeywords: $nxthread $os
//===============================================================================//

#ifdef __SWITCH__

#ifndef HORIZONTHREAD_H
#define HORIZONTHREAD_H

#include "Thread.h"

#include <switch.h>

class HorizonThread : public BaseThread
{
public:
	HorizonThread(McThread::START_ROUTINE start_routine, void *arg);
	virtual ~HorizonThread();

	bool isReady() {return m_bReady;}

private:
	bool m_bReady;

	Thread m_thread;
};

#endif

#endif
