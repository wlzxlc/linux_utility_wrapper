/*
 *  Copyright (c) 2014 The linux_utility_wrapper project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ccstone/log.h>
#define LOG_BUF_SIZE 1024
#ifndef NULL
#define NULL 0
#endif
#ifdef ANDROID_LOG
#include <android/log.h>
#endif

static trace_log_write trace_log_fun;

#ifndef ANDROID_LOG
static int __trace_log_write(int prio, const char *tag, const char *msg){
	int ret = -1;
	if (prio >= TRACE_LOG_UNKNOWN && prio <= TRACE_LOG_SILENT) {
		static const char vtab[TRACE_LOG_SILENT + 1] = { 'U', 'd', 'V', 'D',
				'I', 'W', 'E', 'F', 'S' };
		ret = printf("%c/%s: %s\n", vtab[prio], tag ? tag : "", msg ? msg : "");
	}
	return ret;
}
#endif
int __trace_log_redirect(trace_log_write pfun){
	trace_log_fun = pfun;
	return 0;
}

static int __trace_internal_write(int prio, const char *tag,
		const char *msg) {
	trace_log_write pfun = NULL;
#ifdef ANDROID_LOG
	pfun = trace_log_fun ?trace_log_fun: __android_log_write;
#else
	pfun = trace_log_fun ? trace_log_fun : __trace_log_write;
#endif
	return (*pfun)(prio, tag, msg);
}

int __trace_log_print(int prio, const char *tag, const char *fmt, ...) {
	va_list ap;
	char buf[LOG_BUF_SIZE];
	va_start(ap, fmt);
	vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
	va_end(ap);
	return __trace_internal_write(prio, tag, buf);
}
void __trace_log_assert(const char *cond, const char *tag, const char *fmt, ...) {
	char buf[LOG_BUF_SIZE];

	if (fmt) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
		va_end(ap);
	} else {
		/* Msg not provided, log condition.  N.B. Do not use cond directly as
		 * format string as it could contain spurious '%' syntax (e.g.
		 * "%d" in "blocks%devs == 0").
		 */
		if (cond)
			snprintf(buf, LOG_BUF_SIZE, "Assertion failed: %s", cond);
		else
			strcpy(buf, "Unspecified assertion failed");
	}
	(void) (__trace_internal_write(TRACE_LOG_FATAL, tag, buf));
	__builtin_trap(); /* trap so we have a chance to debug the situation */
}
