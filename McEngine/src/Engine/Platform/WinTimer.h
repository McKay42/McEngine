//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $wintime
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifndef WINTIMER_H
#define WINTIMER_H

#include "Timer.h"

#include "BaseTsd.h"
#include <Windows.h>

class WinTimer : public BaseTimer
{
public:
	WinTimer();

    void start();
    void update();

    void sleep(unsigned int us);

    inline double getDelta() const {return m_delta;}
    inline double getElapsedTime() const {return m_elapsedTime;}

private:
    double m_ticksPerSecond;
    LARGE_INTEGER m_currentTime;
    LARGE_INTEGER m_startTime;

    double m_delta;
    double m_elapsedTime;
};

#endif

#endif
