//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $nxtime
//===============================================================================//

#ifdef __SWITCH__

#ifndef HORIZONTIMER_H
#define HORIZONTIMER_H

#include "cbase.h"

#include "Timer.h"

class HorizonTimer : public BaseTimer
{
public:
	HorizonTimer();
	virtual ~HorizonTimer();

	void start();
	void update();

	void sleep(unsigned int us);

	inline double getDelta() const {return m_delta;}
	inline double getElapsedTime()  const {return m_elapsedTime;}

private:
    uint64_t m_startTime;
    uint64_t m_currentTime;

    double m_delta;
    double m_elapsedTime;
};

#endif

#endif
