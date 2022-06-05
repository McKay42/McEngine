//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $mactime $os
//===============================================================================//

#ifdef __APPLE__

#include "MacOSTimer.h"

MacOSTimer::MacOSTimer() : BaseTimer()
{
	mach_timebase_info(&m_timebaseInfo);

	m_currentTime = 0;
	m_startTime = 0;

	m_delta = 0.0;
	m_elapsedTime = 0.0;
	m_elapsedTimeMS = 0;
}

void MacOSTimer::start()
{
	m_startTime = mach_absolute_time();
	m_currentTime = m_startTime;

	m_delta = 0.0;
	m_elapsedTime = 0.0;
	m_elapsedTimeMS = 0;
}

void MacOSTimer::update()
{
	const uint64_t nowTime = mach_absolute_time();

	m_delta = (((nowTime - m_currentTime) * m_timebaseInfo.numer) / m_timebaseInfo.denom) / 1000000000.0;
	m_elapsedTime = (((nowTime - m_startTime) * m_timebaseInfo.numer) / m_timebaseInfo.denom) / 1000000000.0;
	m_elapsedTimeMS = (((nowTime - m_startTime) * (uint64_t)m_timebaseInfo.numer) / (uint64_t)m_timebaseInfo.denom) / (uint64_t)1000000;
	m_currentTime = nowTime;
}

#endif
