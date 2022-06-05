//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $wintime $os
//===============================================================================//

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)

#ifndef WINTIMER_H
#define WINTIMER_H

#include "Timer.h"

//#define WIN32_LEAN_AND_MEAN
//#define NOCRYPT
#include "BaseTsd.h"
#include <Windows.h>

class WinTimer : public BaseTimer
{
public:
	WinTimer();
	virtual ~WinTimer() {;}

	virtual void start() override;
	virtual void update() override;

	virtual inline double getDelta() const override {return m_delta;}
	virtual inline double getElapsedTime() const override {return m_elapsedTime;}
	virtual inline uint64_t getElapsedTimeMS() const override {return m_elapsedTimeMS;}

private:
	double m_secondsPerTick;
	LONGLONG m_ticksPerSecond;

	LARGE_INTEGER m_currentTime;
	LARGE_INTEGER m_startTime;

	double m_delta;
	double m_elapsedTime;
	uint64_t m_elapsedTimeMS;
};

#endif

#endif
