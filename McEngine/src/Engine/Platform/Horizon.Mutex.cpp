//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		partial std::mutex implementation for libnx (see mingw.mutex.h)
//
// $NoKeywords: $nxmutex
//===============================================================================//

#ifdef __SWITCH__

#include "Horizon.Mutex.h"

#include <switch.h>

class HorizonMutexWrapper
{
public:
	Mutex handle;
};

class HorizonRMutexWrapper
{
public:
	RMutex handle;
};

namespace std
{
	recursive_mutex::recursive_mutex() noexcept
	{
		m_wrapper = new HorizonRMutexWrapper();
		rmutexInit(&m_wrapper->handle);
	}

	recursive_mutex::~recursive_mutex() noexcept
	{
		if (m_wrapper != NULL)
			delete m_wrapper;

		m_wrapper = NULL;
	}

	void recursive_mutex::lock()
	{
		rmutexLock(&m_wrapper->handle);
	}

	void recursive_mutex::unlock()
	{
		rmutexUnlock(&m_wrapper->handle);
	}

	bool recursive_mutex::try_lock()
	{
		return rmutexTryLock(&m_wrapper->handle);
	}

	mutex::mutex() noexcept
	{
		m_wrapper = new HorizonMutexWrapper();
		mutexInit(&m_wrapper->handle);
	}

	mutex::~mutex() noexcept
	{
		if (m_wrapper != NULL)
			delete m_wrapper;

		m_wrapper = NULL;
	}

	void mutex::lock()
	{
		mutexLock(&m_wrapper->handle);
	}

	void mutex::unlock()
	{
		mutexUnlock(&m_wrapper->handle);
	}

	bool mutex::try_lock()
	{
		return mutexTryLock(&m_wrapper->handle);
	}
}

#endif
