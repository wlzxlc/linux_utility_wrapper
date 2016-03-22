#include <ccstone/telnetd/osptele.h>
static int command(int a1,int a2,int a3,int a4){
	char buffer[100] = {0};
	sprintf(buffer, "command arg1 %d arg2 %d arg3 %d arg4 %d\n",a1,a2,a3,a4);
	TelePrint(buffer);
    return 0;
}
static void command2(int a1){
	OspPrintf(TRUE,TRUE,"%s %d\n",__func__,a1);
	return;
}
struct AA{
	int a;
	int c;
	int d;
};
static void command3(struct AA a){
	OspPrintf(TRUE,TRUE,"%s a %d \n",__func__,a.a);
	return;
}
int main(int c, char **s)
{
   int port = 3389;
  #ifdef TELNETD_PORT
   port = TELNETD_PORT;
  #endif
  OspTelAuthor("kedacom","kedacom");
  OspRegCommand("cmd1",command,"\n cmd1 [arg1] [arg2] [arg3] [arg4]\n");
  OspRegCommand("cmd2",command2,"\n cmd2 [arg1]\n");
  OspRegCommand("cmd3",command3,"\n cmd3 [arg1]\n");
  OspTeleDaemon(port, NULL);
  printf("exit main\n"); 
  return 0;
}
