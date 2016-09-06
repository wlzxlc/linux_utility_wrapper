#define LOG_TAG "TestLog"
#define LOG_NDEBUG 0
#include <ccstone/defines.h>
#include <ccstone/logd.h>
#include <ccstone/thread.h>
#include <unistd.h>
int main(int argc, char **args)
{
  SCOPEDDEBUG();
  DEBUG("%s", "Debug");
  ALOGV("ALOGV");
  ALOGD("ALOGD");
  ALOGI("ALOGI");
  ALOGW("ALOGW");
  ALOGE("ALOGE");

  __trace_set_log_file("trace_log.txt", 1024 * 1024);
  __trace_set_log_level(TRACE_LOG_WARN);
  DEBUG("%s", "Debug");
  ALOGV("TEST ALOGV");
  ALOGD("TEST ALOGD");
  ALOGI("TEST ALOGI");
  ALOGW("TEST ALOGW");
  ALOGE("TSET ALOGE");
  __trace_set_log_level(TRACE_LOG_DEFAULT);
  DEBUG("trace log.");
  {
	  SCOPEDDEBUG();
  }
  __trace_set_log_file(NULL, 0);
  return 0;
}
