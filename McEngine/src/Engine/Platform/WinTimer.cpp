//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $wintime
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinTimer.h"

WinTimer::WinTimer()
{
	m_delta = 0;
	m_elapsedTime = 0;

	LARGE_INTEGER ticks;
	QueryPerformanceFrequency( &ticks );
	m_ticksPerSecond = (double)ticks.QuadPart;
}

void WinTimer::start()
{
	QueryPerformanceCounter(&m_startTime);
}

void WinTimer::update()
{
	// get the current time
	LARGE_INTEGER nowTime;
	QueryPerformanceCounter( &nowTime );

	// update timer
	m_elapsedTime = ((double)nowTime.QuadPart - (double)m_startTime.QuadPart) / m_ticksPerSecond;
	m_delta = (double)((nowTime.QuadPart - m_currentTime.QuadPart) / m_ticksPerSecond);
	m_currentTime = nowTime;
}

void WinTimer::sleep(unsigned int us)
{
	Sleep(us/1000);
}

#endif
