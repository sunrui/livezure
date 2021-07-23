#pragma once

#include <stdafx.h>

class MutexLock
{
public:
	MutexLock() { InitializeCriticalSection(&cs); }
	~MutexLock() { DeleteCriticalSection(&cs); }

	class AutoLock
	{
	public:
		AutoLock(MutexLock * lock) : m_lock(lock) { lock->Lock(); }
		~AutoLock() { m_lock->Unlock(); }

	private:
		MutexLock * m_lock;
	};

protected:
	void Lock() { EnterCriticalSection(&cs); }
	void Unlock() { LeaveCriticalSection(&cs); }

private:
	CRITICAL_SECTION cs;
};