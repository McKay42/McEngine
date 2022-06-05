//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $time
//===============================================================================//

#ifndef TIMER_H
#define TIMER_H

#include "cbase.h" // for uint64_t

class BaseTimer
{
public:
	virtual ~BaseTimer() {;}

	virtual void start() = 0;
	virtual void update() = 0;

	virtual double getDelta() const = 0;
	virtual double getElapsedTime() const = 0;
	virtual uint64_t getElapsedTimeMS() const = 0;
};

class Timer
{
public:
	Timer();
	~Timer();

	inline void start() {m_timer->start();}
	inline void update() {m_timer->update();}

	inline double getDelta() const {return m_timer->getDelta();}
	inline double getElapsedTime() const {return m_timer->getElapsedTime();}
	inline uint64_t getElapsedTimeMS() const {return m_timer->getElapsedTimeMS();}

private:
	BaseTimer *m_timer;
};

#endif
