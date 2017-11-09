#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "rwlock.h"

/* rwl implements a reader-writer lock.
 * A reader-write lock can be acquired in two different modes, 
 * the "read" (also referred to as "shared") mode,
 * and the "write" (also referred to as "exclusive") mode.
 * Many threads can grab the lock in the "read" mode.  
 * By contrast, if one thread has acquired the lock in "write" mode, no other 
 * threads can acquire the lock in either "read" or "write" mode.
 */

//helper function
static inline int
cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *expire)
{
	int r; 
	if (expire != NULL)  {
		r = pthread_cond_timedwait(c, m, expire);
	} else
		r = pthread_cond_wait(c, m);
	assert(r == 0 || r == ETIMEDOUT);
       	return r;
}

//rwl_init initializes the reader-writer lock 
void
rwl_init(rwl *l)
{
	//Your code here
	pthread_mutex_init(&(l->rwlock), NULL); //initialize the lock
	pthread_cond_init(&(l->rwcond), NULL); //initialize the condition varialble
	l->readers = 0; //set the number of readers grabbing the lock to zero
	l->writer = 0; //set if the writer is grabbing the lock to false
	l->askread = 0; //set the number of readers waiting for the lock to zero
	l->askwrite = 0; //set the number of writers waiting for the lock to zero

}

//rwl_nwaiters returns the number of threads *waiting* to acquire the lock
//Note: it should not include any thread who has already grabbed the lock
int
rwl_nwaiters(rwl *l) 
{
	//Your code here
	pthread_mutex_lock(&(l->rwlock));
	int waiters = l->askwrite+l->askread;
	pthread_mutex_unlock(&(l->rwlock));
	return waiters;
	
}

//rwl_rlock attempts to grab the lock in "read" mode
//if lock is not grabbed before absolute time "expire", it returns ETIMEDOUT
//else it returns 0 (when successfully grabbing the lock)
int
rwl_rlock(rwl *l, const struct timespec *expire)
{
	//Your code here
	pthread_mutex_lock(&(l->rwlock));
	int wait;
	while (l->writer || l->askwrite){ //wait giving writer priority
		l->askread ++;
		wait = cond_timedwait(&(l->rwcond), &(l->rwlock), expire);
		if (wait == ETIMEDOUT) {
			l->askread --; //reader stops waiting since timedout
			pthread_mutex_unlock(&(l->rwlock));
			return ETIMEDOUT;
		}
		l->askread --; //reader grabs the lock and is no longer waiting
	}
	l->readers++; //the number of reader grabbing the lock increment by 1
	pthread_mutex_unlock(&(l->rwlock));
	return 0;
}

//rwl_runlock unlocks the lock held in the "read" mode
void
rwl_runlock(rwl *l)
{
	//Your code here
	pthread_mutex_lock(&(l->rwlock));
	l->readers--; //number of reader grabbing the lock decrement by 1
	/* if no more reader is grabbing the lock, then broadcast the lock
	 * as available */
	if (l->readers == 0 && l->writer ==0) //
		pthread_cond_broadcast(&(l->rwcond));
	pthread_mutex_unlock(&(l->rwlock));
}

//rwl_wlock attempts to grab the lock in "write" mode
//if lock is not grabbed before absolute time "expire", it returns ETIMEDOUT
//else it returns 0 (when successfully grabbing the lock)
int
rwl_wlock(rwl *l, const struct timespec *expire)
{
	//Your code here
	pthread_mutex_lock(&(l->rwlock));
	int wait;
	while (l->writer || (l->readers > 0)) { //wait until no one grabbing the lock
		l->askwrite ++;
		wait = cond_timedwait(&(l->rwcond), &(l->rwlock), expire);
		if (wait == ETIMEDOUT) {
			l->askwrite --;
			pthread_mutex_unlock(&(l->rwlock));
			return ETIMEDOUT;
		}
		l->askwrite --;
	}
	l->writer = 1;
	pthread_mutex_unlock(&(l->rwlock));
	return 0;
}

//rwl_wunlock unlocks the lock held in the "write" mode
void
rwl_wunlock(rwl *l)
{
	//Your code here
	pthread_mutex_lock(&(l->rwlock));
	l->writer = 0;
	pthread_cond_broadcast(&(l->rwcond));
	pthread_mutex_unlock(&(l->rwlock));
}
