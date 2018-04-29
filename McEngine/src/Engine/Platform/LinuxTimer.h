//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $linuxtime
//===============================================================================//

#ifdef __linux__

#ifndef LINUXTIMER_H
#define LINUXTIMER_H

#include "Timer.h"

#include <time.h>

class LinuxTimer : public BaseTimer
{
public:
	LinuxTimer();

    void start();
    void update();

    void sleep(unsigned int us);

    inline double getDelta() const {return m_delta;}
    inline double getElapsedTime() const {return m_elapsedTime;}

private:
    timespec m_startTime;
    timespec m_currentTime;

    double m_delta;
    double m_elapsedTime;
};

#endif

#endif
