//================ Copyright (c) 2020, PG, All rights reserved. =================//
//
// Purpose:		thread wrapper
//
// $NoKeywords: $thread
//===============================================================================//

#ifndef THREAD_H
#define THREAD_H

class ConVar;

class BaseThread
{
public:
	virtual ~BaseThread() {;}

	virtual bool isReady() = 0;
};

class McThread
{
public:
	static ConVar *debug;

	typedef void *(*START_ROUTINE)(void*);

public:
	McThread(START_ROUTINE start_routine, void *arg);
	~McThread();

	bool isReady();

private:
	BaseThread *m_baseThread;
};

#endif
