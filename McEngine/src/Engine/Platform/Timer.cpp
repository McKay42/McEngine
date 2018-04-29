//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $time $os
//===============================================================================//

#include "Timer.h"
#include "cbase.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#include "WinTimer.h"

#elif defined __linux__

#include "LinuxTimer.h"

#elif defined __APPLE__

#include "MacOSTimer.h"

#endif

Timer::Timer()
{
	m_timer = NULL;

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

	m_timer = new WinTimer();

#elif defined __linux__

	m_timer = new LinuxTimer();

#elif defined __APPLE__

	m_timer = new MacOSTimer();

#else
#error Missing Timer implementation for OS!
#endif

}

Timer::~Timer()
{
	SAFE_DELETE(m_timer);
}

void Timer::start()
{
	m_timer->start();
}

void Timer::update()
{
	m_timer->update();
}

void Timer::sleep(unsigned int us)
{
	m_timer->sleep(us);
}

