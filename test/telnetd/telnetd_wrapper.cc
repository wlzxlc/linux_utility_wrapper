#include <assert.h>
#include "telnetd_wrapper.h"
#include <ccstone/telnetd/osptele.h>
#include <stdarg.h>
#include <unistd.h>
#ifndef MAX_MESSAGE_SIZE
#undef MAX_MESSAGE_SIZE
#define MAX_MESSAGE_SIZE 1024
#endif

int Telnetd::OspUniformFuncCMD(const char *cmd,const char *args[],int size ,void  *priv){
	Telnetd *telnetd = (Telnetd *)priv;
	int ret = 0;
	if (telnetd){
		ret = telnetd->TelnetdHandle(cmd,args,size);
	}
	return ret;
}
Telnetd::Telnetd():_init(false),_threadName(NULL){
	_table.clear();
}
 int Telnetd::Init(int16_t port, const char *threadName, const char *author,
 	const char *password ){
	if (!_init){
		_port = port ? port:3389;
		_author = author;
		_password = password;
        _threadName = threadName;
        setName(threadName);
        CHECK_GT((int)start(), 0);
	}
		return 0;
 }
 bool Telnetd::run(){
 	OspUniformFuncRedirect(Telnetd::OspUniformFuncCMD,this);
	printf("Telned start\n");
 	OspTeleDaemon(_port, _threadName);
 	return false;
}
 int Telnetd::TelnetdHandle(const char *cmd,const char *args[], int size){
	TelnetdCmdHandle *handle = NULL;
	int ret = -1;
 	if (cmd){
	 	CCStone::AutoLock autoLock(&_lock);
		std::list<_CmdTable>::iterator item = _table.end();
		for (item = _table.begin(); item != _table.end(); item ++ ){
				if ((*item).cmd == std::string(cmd)){
					handle = (*item).handle;
					break;
				}
		}
	}
	if (handle){
        printf("cmd %s arg1 %s arg2 %s arg3 %s size %d handle %p\n",cmd,args[0],args[1],args[2],size,handle);
	    ret = handle->Process(args,size);
	}
	return ret;
 }
 int Telnetd::RequestExit(){
     stop();
     waitExit();
     OspExit();
	return 0;
 }
 int Telnetd::RegisterCommand(const char *cmd, const char * usage, TelnetdCmdHandle *handle){
 	if (cmd && handle && _table.size() < RegCMD_TABLE_SIZE){
	 	CCStone::AutoLock autoLock(&_lock);
		OspRegCommand(cmd,NULL,usage);
		_table.push_back(_CmdTable(cmd,handle));
 		}
	return 0;
 }
 int Telnetd::DeRegisterCommand(const char *cmd){
 	if (cmd){
	 	CCStone::AutoLock autoLock(&_lock);
		std::list<_CmdTable>::iterator item = _table.end();
		for (item = _table.begin(); item != _table.end(); item ++ ){
				if ((*item).cmd == std::string(cmd)){
					_table.erase(item);
					return 0;
				}
			}
 		}
	return -1;
 }
 int Telnetd::Print(const char *fmt, ...){
 	 char temp_buff[MAX_MESSAGE_SIZE]= {0};
	 char *buff = NULL;
	if (fmt) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(temp_buff, MAX_MESSAGE_SIZE - 1, fmt, args);
        va_end(args);
        buff = temp_buff;
      }
	if (buff){
		TelePrint(buff);
	}
	return 0;
 }
 Telnetd::~Telnetd(){
	RequestExit();
 }
