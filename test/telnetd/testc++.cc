#include "telnetd_wrapper.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

class TelnetdCmdHandleImpl:public TelnetdCmdHandle {
private:
		int aa;
		Telnetd *telnetd;
public:
	TelnetdCmdHandleImpl(int a,Telnetd *t){
		aa = a;
		telnetd = t;
	}
	virtual int Process(const char *a1[], int size){
        if (size)
		aa += atoi(a1[0]);
        telnetd->Print("aa %d\n",aa);
		return 0;
	}
};


int main(int a,char **ss){
	Telnetd telnetd;
	assert(telnetd.Init(3322,"mytelnetd")>=0);	
    TelnetdCmdHandle *handle = new TelnetdCmdHandleImpl(12,&telnetd);
    printf("%p\n",handle);
	telnetd.RegisterCommand("testcmd1","testcmd1 arg1 arg2 arg3\n",handle);
	while(1) usleep(10000);
}
