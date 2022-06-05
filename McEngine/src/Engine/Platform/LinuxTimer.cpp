//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $linuxtime $os
//===============================================================================//

#ifdef __linux__

#include "LinuxTimer.h"

static timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec - start.tv_nsec) < 0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}

LinuxTimer::LinuxTimer() : BaseTimer()
{
	m_startTime.tv_sec = 0;
	m_startTime.tv_nsec = 0;
	m_currentTime.tv_sec = 0;
	m_currentTime.tv_nsec = 0;

	m_delta = 0.0;
	m_elapsedTime = 0.0;
	m_elapsedTimeMS = 0;
}

void LinuxTimer::start()
{
	clock_gettime(CLOCK_REALTIME, &m_startTime);
	m_currentTime = m_startTime;

	m_delta = 0.0;
	m_elapsedTime = 0.0;
	m_elapsedTimeMS = 0;
}

void LinuxTimer::update()
{
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);

	const timespec delta = diff(m_currentTime, t);
	m_delta = delta.tv_sec + (double)delta.tv_nsec / 1000000000.0;

	const timespec elapsed = diff(m_startTime, t);
	m_elapsedTime = elapsed.tv_sec + (double)elapsed.tv_nsec / 1000000000.0;
	m_elapsedTimeMS = ((uint64_t)elapsed.tv_sec * (uint64_t)1000) + ((uint64_t)elapsed.tv_nsec / (uint64_t)1000000);

	m_currentTime = t;
}

#endif
