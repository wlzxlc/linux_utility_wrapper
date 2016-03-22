#define LOG_TAG "TestLog"
#define LOG_NDEBUG 0
#include <ccstone/defines.h>
#include <ccstone/logd.h>

int main(int argc, char **args)
{
  SCOPEDDEBUG();
  ALOGV("ALOGV");
  ALOGD("ALOGD");
  ALOGI("ALOGI");
  ALOGW("ALOGW");
  ALOGE("ALOGE");

  status err = ERRNUM(123);
  CHECK_EQ(123, CSM_CUSTOM(err));
  CHECK_EQ(__LINE__ - 2, CSM_LINE(err));
  err = ERRNUMSTR("init err value.");
  DEBUG("err %d", err);
  return err;
}
