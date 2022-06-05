//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $nxtime $os
//===============================================================================//

#ifdef __SWITCH__

#ifndef HORIZONTIMER_H
#define HORIZONTIMER_H

#include "Timer.h"

class HorizonTimer : public BaseTimer
{
public:
	HorizonTimer();
	virtual ~HorizonTimer() {;}

	virtual void start() override;
	virtual void update() override;

	virtual inline double getDelta() const override {return m_delta;}
	virtual inline double getElapsedTime() const override {return m_elapsedTime;}
	virtual inline uint64_t getElapsedTimeMS() const override {return m_elapsedTimeMS;}

private:
    uint64_t m_startTime;
    uint64_t m_currentTime;

    double m_delta;
    double m_elapsedTime;
    uint64_t m_elapsedTimeMS;
};

#endif

#endif
