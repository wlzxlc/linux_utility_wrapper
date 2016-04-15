#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef HAVE_THREAD
#include <pthread.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <malloc.h>
#include <inttypes.h>
#include "common_type.h"
#include "defines.h"
#ifdef __cplusplus
namespace CCStone {
class Mutex {
    private:
        friend class Condition;
        pthread_mutex_t _mutex;
        pthread_mutexattr_t _attr;
    public:
        Mutex(){
            pthread_mutexattr_init(&_attr);
            pthread_mutex_init(&_mutex, &_attr);
        }

        ~Mutex()
        {
           pthread_mutexattr_destroy(&_attr);
           pthread_mutex_destroy(&_mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
};

class AutoLock {
    private:
        Mutex *_mutex;
    public:
        AutoLock(Mutex *lock):_mutex(lock)
    {
        _mutex->lock();
    }
        ~AutoLock()
        {
            _mutex->unlock();
        }
};

class Condition {
    private:
        pthread_condattr_t _attr;
        pthread_cond_t _cond;
    public:
        Condition() {
            pthread_condattr_init(&_attr);
            pthread_cond_init(&_cond, &_attr);
        }

        ~Condition()
        {
            pthread_condattr_destroy(&_attr);
            pthread_cond_destroy(&_cond);
        }

        int wait(Mutex *mutex) {
            return pthread_cond_wait(&_cond, &mutex->_mutex);
        }

        int signal() {
            return pthread_cond_signal(&_cond);
        }

        int broadcast() {
            return pthread_cond_broadcast(&_cond);
        }

        int timedWait(Mutex *mutex, uint64_t ms) {
            struct timespec ts;
            struct timeval t;
            gettimeofday(&t, NULL);
            ts.tv_sec = t.tv_sec;
            ts.tv_nsec= t.tv_usec*1000;
            ts.tv_sec += ms / 1000;
            ts.tv_nsec += (ms % 1000) * 1E9;
            return pthread_cond_timedwait(&_cond,&mutex->_mutex, &ts);
        }
};

class Thread {
    private:
        bool _run;
        bool _alive;
        char *_name;
        uint32_t _tid;
        pthread_attr_t _attr;
        pthread_t _thread;
        Mutex _mutex;
        Condition _cond;
    private:
        static void *sRun(void *p)
        {
            SCOPEDDEBUG();
            Thread *self = static_cast<Thread *>(p);
            prctl(PR_SET_NAME, (unsigned long)self->_name, 0, 0, 0);

            self->_tid = static_cast<unsigned int> ((syscall(__NR_gettid)));


            {
                AutoLock lock(&(self->_mutex));
                self->_alive = true;
                self->_run = true;
                self->_cond.broadcast();
            }

            ALOG(LOG_DEBUG, "Thread", "Thread[%u] with name:%s started ",
                    self->_tid, self->_name);

            while(self->_alive && self->run());

            ALOG(LOG_DEBUG, "Thread", "Thread[%u] with name:%s stoped ",
                    self->_tid, self->_name);
            {
                AutoLock lock(&(self->_mutex));
                self->_run = false;
                self->_cond.broadcast();
            }

            return NULL;
        }

    protected:
        virtual bool run() = 0;
    public:
        Thread(const char *name = NULL):
           _run(false),
           _alive(false),
           _name(NULL),
          _tid(0) 
        {
            setName(name);
        }

        status_t setName(const char *name) 
        {
            AutoLock lock(&_mutex);
            if (_run) {
                return ERRNUMSTR("Thread already run.");
            }

            int size = 0;
            if ( NULL == name ) {
                name = "CCStoneThread";
            }
            size = strlen(name);
            if (_name) {
                free(_name);
            }
            _name = (char *)malloc(size + 1);
            memset(_name, 0, size + 1);
            memcpy(_name, name, size);
            return 0;
        }

       ~Thread()
        {
          stop(false);
          free(_name);
        }

        uint32_t start()
        {
            int result = 0;
            AutoLock lock(&_mutex);
            result = pthread_attr_init(&_attr);
            if (result != 0) {
                return ERRNUMSTR("Thread init error.");
            }

            // Set Sched mode RR/FIFO
            result = pthread_attr_setschedpolicy(&_attr,SCHED_RR);

            // Detached this thread
            //result |= pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED);

            // Set the stack stack size to 1M.
            result |= pthread_attr_setstacksize(&_attr, 1024 * 1024);

            if (result) {
                ALOG(LOG_WARN, "Thread", "Set thread attribute have a error.");
            }

            result = pthread_create(&_thread, &_attr, Thread::sRun, this);

            while( 0 == result && (!_run)) {
                _cond.timedWait(&_mutex, 1000);
            }
            return _tid;
        }

        status_t stop(bool sync = false, uint64_t ms = 0) 
        {
            AutoLock lock(&_mutex);
            _alive = false;
            if(sync && _run) {
             _cond.timedWait(&_mutex, ms);
            }
            return _run ? ERRNUM(0) : 0;
        }

        status_t waitExit(uint64_t ms = 0x7FFFFFFF) 
        {
            AutoLock lock(&_mutex);
            if (_run) {
             ALOGD("Wait %"PRIu64 "(ms)", ms);
             SCOPEDDEBUG();
             _cond.timedWait(&_mutex, ms);
            }
            return _run ? ERRNUM(0) : 0;
        } 

};

} /*end of the CCStone*/
#endif /*end of __cplusplus */
#else
 #error Undefined HAVE_THREAD macro.
#endif /* HAVE_THREAD*/
#endif
