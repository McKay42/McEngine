//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $nxtime $os
//===============================================================================//

#ifdef __SWITCH__

#include "HorizonTimer.h"

#include <switch.h>

HorizonTimer::HorizonTimer() : BaseTimer()
{
	m_startTime = 0;
	m_currentTime = 0;

	m_delta = 0.0;
	m_elapsedTime = 0.0;
	m_elapsedTimeMS = 0;
}

void HorizonTimer::start()
{
	m_startTime = armGetSystemTick();
	m_currentTime = m_startTime;

	m_delta = 0.0;
	m_elapsedTime = 0.0;
	m_elapsedTimeMS = 0;
}

void HorizonTimer::update()
{
	const uint64_t nowTime = armGetSystemTick();

	m_delta = (double)armTicksToNs(nowTime - m_currentTime) / 1000000000.0;
	m_elapsedTime = (double)armTicksToNs(nowTime - m_startTime) / 1000000000.0;
	m_elapsedTimeMS = armTicksToNs(nowTime - m_startTime) / (uint64_t)1000000;
	m_currentTime = nowTime;
}

#endif
