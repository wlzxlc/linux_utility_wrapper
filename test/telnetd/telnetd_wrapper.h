#ifndef TELNETD_H_
#define TELNETD_H_
#include "critical_section_wrapper.h"
#include "scoped_ptr.h"
#include "thread_wrapper.h"
#include <string>
#include <list>
class TelnetdCmdHandle{
public:
	virtual int Process(const char *args[], int size);
};

class Telnetd{
private:
	webrtc::scoped_ptr<webrtc::ThreadWrapper> _thread;
	webrtc::scoped_ptr<webrtc::CriticalSectionWrapper> _lock;
	bool _init;
	const char *_author;
	const char *_password;
    const char *_threadName;
	int16_t _port;
	struct _CmdTable{
		_CmdTable(const char *cstr, TelnetdCmdHandle *hand){
			cmd = std::string(cstr);
			handle = hand;
		}
		std::string cmd;
		TelnetdCmdHandle *handle;
		};
	std::list<_CmdTable> _table;
private:
	bool Process();
	int TelnetdHandle(const char *cmd, const char *args[], int size);
	static bool _thread_process(void *p);
	static int OspUniformFuncCMD(const char *cmd,const char *args[], int size, void  *priv);
public:
 Telnetd();
  int Init(int16_t port, const char *threadName, const char *author = NULL, const char *password = NULL);
  int RequestExit();
  int RegisterCommand(const char *cmd, const char * usage, TelnetdCmdHandle *handle);
  int DeRegisterCommand(const char *cmd);
  int Print(const char *fmt, ...);
  ~Telnetd();
};
#endif
