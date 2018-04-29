//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $mactime
//===============================================================================//

#ifdef __APPLE__

#include "MacOSTimer.h"

#include <unistd.h>

MacOSTimer::MacOSTimer()
{
	m_delta = 0;
	m_elapsedTime = 0;

	m_currentTime = 0;
	m_startTime = 0;

	mach_timebase_info(&m_timebaseInfo);
}

MacOSTimer::~MacOSTimer()
{
}

void MacOSTimer::start()
{
	m_startTime = mach_absolute_time();
	m_currentTime = m_startTime;
}

void MacOSTimer::update()
{
	// get current time
	uint64_t nowTime = mach_absolute_time();

	// update timer
	m_elapsedTime = (((nowTime - m_startTime) * m_timebaseInfo.numer) / m_timebaseInfo.denom) / 1000000000.0;
	m_delta = (((nowTime - m_currentTime) * m_timebaseInfo.numer) / m_timebaseInfo.denom) / 1000000000.0;
	m_currentTime = nowTime;
}

void MacOSTimer::sleep(unsigned int us)
{
	usleep(us);
}

#endif
