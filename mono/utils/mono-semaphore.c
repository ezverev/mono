/*
 * mono-semaphore.c: mono-semaphore functions
 *
 * Author:
 *	Gonzalo Paniagua Javier  <gonzalo@novell.com>
 *
 * (C) 2010 Novell, Inc.
 */

#include <config.h>
#include <errno.h>
#include "utils/mono-semaphore.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if (defined(HAVE_SEMAPHORE_H) || defined(USE_MACH_SEMA))
/* sem_* or semaphore_* functions in use */
#  ifdef USE_MACH_SEMA
#    define TIMESPEC mach_timespec_t
#    define WAIT_BLOCK(a,b) semaphore_timedwait (*(a), *(b))
#  elif defined(__OpenBSD__)
#    define TIMESPEC struct timespec
#    define WAIT_BLOCK(a) sem_trywait(a)
#  else
#    define TIMESPEC struct timespec
#    define WAIT_BLOCK(a,b) sem_timedwait (a, b)
#  endif

int
mono_sem_timedwait (MonoSemType *sem, guint32 timeout_ms, gboolean alertable)
{
	TIMESPEC ts, copy;
	struct timeval t;
	int res = 0;
#if defined(__OpenBSD__)
	int timeout;
#endif

#ifndef USE_MACH_SEMA
	if (timeout_ms == 0)
		return (!sem_trywait (sem));
#endif
	if (timeout_ms == (guint32) 0xFFFFFFFF)
		return mono_sem_wait (sem, alertable);

	gettimeofday (&t, NULL);
	ts.tv_sec = timeout_ms / 1000 + t.tv_sec;
	ts.tv_nsec = (timeout_ms % 1000) * 1000000 + t.tv_usec * 1000;
	while (ts.tv_nsec > 1000000000) {
		ts.tv_nsec -= 1000000000;
		ts.tv_sec++;
	}
#if defined(__OpenBSD__)
	timeout = ts.tv_sec;
	while (timeout) {
		if ((res = WAIT_BLOCK (sem)) == 0)
			return res;

		if (alertable)
			return -1;

		usleep (ts.tv_nsec / 1000);
		timeout--;
	}
#else
	copy = ts;
	while ((res = WAIT_BLOCK (sem, &ts) == -1) && errno == EINTR) {
		if (alertable)
			return -1;
		ts = copy;
	}
#endif
	return res;
}

int
mono_sem_wait (MonoSemType *sem, gboolean alertable)
{
	int res;
#ifndef USE_MACH_SEMA
	while ((res = sem_wait (sem) == -1) && errno == EINTR)
#else
	while ((res = semaphore_wait (*sem) == -1) && errno == EINTR)
#endif
	{
		if (alertable)
			return -1;
	}
	return res;
}

int
mono_sem_post (MonoSemType *sem)
{
	int res;
#ifndef USE_MACH_SEMA
	while ((res = sem_post (sem) == -1) && errno == EINTR);
#else
	while ((res = semaphore_signal (*sem) == -1) && errno == EINTR);
#endif
	return res;
}

#else
/* Windows or io-layer functions in use */
int
mono_sem_wait (MonoSemType *sem, gboolean alertable)
{
	return mono_sem_timedwait (sem, INFINITE, alertable);
}

int
mono_sem_timedwait (MonoSemType *sem, guint32 timeout_ms, gboolean alertable)
{
	gboolean res;

	res = WaitForSingleObjectEx (*sem, timeout_ms, alertable);
	switch (res) {
	case WAIT_OBJECT_0:
		return 0;
	case WAIT_IO_COMPLETION:
		errno = EINTR;
		return -1;	      
	// WAIT_TIMEOUT and WAIT_FAILED
	default:
		return -1;
	}
}

int
mono_sem_post (MonoSemType *sem)
{
	if (!ReleaseSemaphore (*sem, 1, NULL))
		return -1;
	return 0;
}
#endif

