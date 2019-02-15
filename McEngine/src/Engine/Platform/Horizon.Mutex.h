//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		partial std::mutex implementation for libnx (see mingw.mutex.h)
//
// $NoKeywords: $nxmutex
//===============================================================================//

#ifdef __SWITCH__

#ifndef HORIZON_MUTEX_H
#define HORIZON_MUTEX_H

class HorizonMutexWrapper;
class HorizonRMutexWrapper;

namespace std
{
	class mutex
	{
	public:
		mutex(const mutex&) = delete;

		mutex() noexcept;
		~mutex() noexcept;

		void lock();
		void unlock();
		bool try_lock();

	protected:
		HorizonMutexWrapper *m_wrapper;
	};

	class recursive_mutex
	{
	public:
		recursive_mutex(const recursive_mutex&) = delete;

		recursive_mutex() noexcept;
		~recursive_mutex() noexcept;

		void lock();
		void unlock();
		bool try_lock();

	protected:
		HorizonRMutexWrapper *m_wrapper;
	};
}

#endif

#endif
