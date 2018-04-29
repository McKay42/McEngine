//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		fps timer
//
// $NoKeywords: $time $os
//===============================================================================//

#ifndef TIMER_H
#define TIMER_H

class BaseTimer
{
public:
	virtual ~BaseTimer() {;}

	virtual void start() = 0;
	virtual void update() = 0;

	virtual void sleep(unsigned int us) = 0;

	virtual double getDelta() const = 0;
	virtual double getElapsedTime()  const = 0;
};

class Timer : public BaseTimer
{
public:
	Timer();
	virtual ~Timer();

    void start();
    void update();

    void sleep(unsigned int us);

    inline double getDelta() const {return m_timer->getDelta();}
    inline double getElapsedTime() const {return m_timer->getElapsedTime();}

private:
    BaseTimer *m_timer;
};


#endif
