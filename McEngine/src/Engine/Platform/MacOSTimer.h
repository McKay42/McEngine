//================ Copyright (c) 2016, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $mactime
//===============================================================================//

#ifdef __APPLE__

#ifndef MACOSTIMER_H
#define MACOSTIMER_H

#include "Timer.h"

#include <mach/mach.h>
#include <mach/mach_time.h>

class MacOSTimer : public BaseTimer
{
public:
	MacOSTimer();
	virtual ~MacOSTimer();

	void start();
	void update();

	void sleep(unsigned int us);

	inline double getDelta() const {return m_delta;}
	inline double getElapsedTime()  const {return m_elapsedTime;}

private:
    mach_timebase_info_data_t m_timebaseInfo;
	uint64_t m_currentTime;
	uint64_t m_startTime;

    double m_delta;
    double m_elapsedTime;
};

#endif

#endif
