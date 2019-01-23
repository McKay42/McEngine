//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $nxtime
//===============================================================================//

#ifdef __SWITCH__

#include "HorizonTimer.h"

#include <switch.h>

HorizonTimer::HorizonTimer()
{
	m_delta = 0;
	m_elapsedTime = 0;

	m_currentTime = 0;
	m_startTime = 0;
}

HorizonTimer::~HorizonTimer()
{
	m_delta = 0;
	m_elapsedTime = 0;
}

void HorizonTimer::start()
{
	m_startTime = armGetSystemTick();
	m_currentTime = m_startTime;
}

void HorizonTimer::update()
{
	// get current time
	const uint64_t nowTime = armGetSystemTick();

	// update timer
	m_elapsedTime = (double)armTicksToNs(nowTime - m_startTime) / 1000000000.0;
	m_delta = (double)armTicksToNs(nowTime - m_currentTime) / 1000000000.0;
	m_currentTime = nowTime;
}

void HorizonTimer::sleep(unsigned int us)
{
	svcSleepThread(us*1000);
}

#endif
