#ifndef _DEFINES_H_
#define _DEFINES_H_
#include "logd.h"
#include <libgen.h>

#ifndef DEBUG
#define DEBUG(fmt, ...) ALOGV("[%s:%d]%s: " fmt, basename(const_cast<char *>(__FILE__)),__LINE__,__FUNCTION__,## __VA_ARGS__)
#endif

#ifdef SCOPEDDEBUG
#undef SCOPEDDEBUG
#endif

#ifdef __cplusplus
#include <string>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

namespace CCStone {
class ScodedDebug {
    public:
        struct timeval _start;
        const char *name;
        int line;
        ScodedDebug(int l, const char *n):
            name(n),
            line(l)
        {
            unsigned tid;
            tid = static_cast<unsigned int> ((syscall(__NR_gettid)));

            gettimeofday(&_start,NULL);
            ALOG(LOG_DEBUG, LOG_TAG, "[ScopedDebug]:%s(), L%d, tid:%u| --> %s (0 ms)",
                    name, line, tid, name);
        }

        ~ScodedDebug()
        {
            struct timeval end;
            unsigned tid;
            tid = static_cast<unsigned int> ((syscall(__NR_gettid)));

            gettimeofday(&end,NULL);
            long long interval_us = 0;
            interval_us = (end.tv_sec - _start.tv_sec)*1E6 + 
                (end.tv_usec - _start.tv_usec);

            ALOG(LOG_DEBUG, LOG_TAG, "[ScopedDebug]:%s(), L%d, tid:%u| <-- %s (%0.2fms)",
                    name, line, tid, name, interval_us / 1000.0);
        }
};

} /*end of the CCStone*/

#define SCOPEDDEBUG() CCStone::ScodedDebug \
     __scopded_debug(__LINE__, __FUNCTION__)

#else
#define SCOPEDDEBUG() 
#endif
#endif
