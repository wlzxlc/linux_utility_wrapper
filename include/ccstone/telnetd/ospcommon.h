#ifndef _OSPCOMMON_H_
#define _OSPCOMMON_H_
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ccstone/logd.h>

typedef unsigned BOOL32;

#define FALSE 0
#define TRUE (!FALSE)
#ifndef NULL
#define NULL 0
#endif

#define API
typedef const char * LPCSTR;

#define MAX_SEM_PER_PROCESS 1024
#define MAX_LOG_MSG_LEN 1024
#define MAX_PROMPT_LEN 21
#define OspLog(level,...) ((void)ALOG(LOG_DEBUG, "Telnetd", __VA_ARGS__))
#define OspDelay(us) usleep(us)
#define STOP_RECV                         0
#define STOP_SEND                         1
#define STOP_BOTH                         2


#endif /* OSPCOMMON_H_ */
