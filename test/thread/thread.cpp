#define LOG_TAG "TestThread"
#define LOG_NDEBUG 0
#include <ccstone/defines.h>
#include <ccstone/logd.h>
#include <ccstone/thread.h>
#include <unistd.h>

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
  CCStone::Mutex lock;
  CCStone::AutoLock autolock(&lock);

  const char *tname = "test_log_thread";
  MyThread thread(tname);
  thread.waitExit(1000 * 2);
  thread.stop(true, 10 * 1000);
  return 0;
}
