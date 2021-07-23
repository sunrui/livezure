/*
 * posix gettimeofday which source from live555
 *
 * @author rui.sun Thu 23:23
 */
#include "posixtime.h"

#ifdef WIN32

#ifndef Boolean
#define Boolean int
#define True 1
#define False 0
#endif

int gettimeofday(struct timeval* tp, int* tz) {
#if defined(_WIN32_WCE)
	/* FILETIME of Jan 1 1970 00:00:00. */
	static const unsigned __int64 epoch = 116444736000000000LL;

	FILETIME    file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
#else
	static LARGE_INTEGER tickFrequency, epochOffset;

	// For our first call, use "ftime()", so that we get a time with a proper epoch.
	// For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.
	static Boolean isFirstCall = True;

	LARGE_INTEGER tickNow;
	QueryPerformanceCounter(&tickNow);

	if (isFirstCall) {
		struct timeb tb;
		ftime(&tb);
		tp->tv_sec = (long)tb.time;
		tp->tv_usec = 1000*tb.millitm;

		// Also get our counter frequency:
		QueryPerformanceFrequency(&tickFrequency);

		// And compute an offset to add to subsequent counter times, so we get a proper epoch:
		epochOffset.QuadPart
			= tb.time*tickFrequency.QuadPart + (tb.millitm*tickFrequency.QuadPart)/1000 - tickNow.QuadPart;

		isFirstCall = False; // for next time
	} else {
		// Adjust our counter time so that we get a proper epoch:
		tickNow.QuadPart += epochOffset.QuadPart;

		tp->tv_sec = (long) (tickNow.QuadPart / tickFrequency.QuadPart);
		tp->tv_usec = (long) (((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);
	}
#endif
	return 0;
}

// ported from php5 code
int usleep(unsigned int useconds)
{
	HANDLE timer;
	LARGE_INTEGER due;

	due.QuadPart = -(10 * (__int64)useconds);

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &due, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
	return 0;
}

#endif

/*
 * count timer sub in milliseconds
 */
int timer_sub(struct timeval * now, struct timeval * last)
{
	int subtv;

	if ((now->tv_usec -= last->tv_usec) < 0)
	{    
		--now->tv_sec;
		now->tv_usec += 1000000;
	}

	now->tv_sec -= last->tv_sec;
	subtv = now->tv_sec * 1000 + now->tv_usec / 1000;

	return subtv;
}