#define LOG_TAG "TestLog"
#define LOG_NDEBUG 0
#include <ccstone/defines.h>
#include <ccstone/logd.h>
#include <ccstone/thread.h>

class MyThread : public CCStone::Thread {
    private:
        int cnt;
 public:
     MyThread(const char *name):Thread(name){ cnt = 500000;}
 protected:
     virtual bool run() {
      ALOGD("Run ...");
      usleep(1000* 1000);
      cnt--;
      return cnt > 0;
     }
};

int main(int argc, char **args)
{
  SCOPEDDEBUG();
  DEBUG("%s", "Debug");
  ALOGV("ALOGV");
  ALOGD("ALOGD");
  ALOGI("ALOGI");
  ALOGW("ALOGW");
  ALOGE("ALOGE");

  status_t err = ERRNUM(123);
  CHECK_EQ(123, CSM_CUSTOM(err));
  CHECK_EQ(__LINE__ - 2, CSM_LINE(err));
  err = ERRNUMSTR("init err value.");
  DEBUG("err %d", err);

  const char *tname = "test_log_thread";
  MyThread thread(tname);
  thread.waitExit(1000 * 2);
  thread.stop(true, 10 * 1000);
  return err;
}
