#include <ccstone/telnetd/osptele.h>
#include "ospsock.h"
#include <stdarg.h>
#ifdef _LINUX_
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>
#include <dlfcn.h>
#include <sys/prctl.h>
#include <stdlib.h>
#endif

SOCKHANDLE g_nSockTelClient = INVALID_SOCKET;
SOCKHANDLE g_nSockTelSer = INVALID_SOCKET;

uint16_t g_wportListtening = 0;
uint32_t g_PromtState = PROMTUSER;
char g_TelnetUsername[AUTHORIZATION_NAME_SIZE]= {0};
char g_TelnetPasswd[AUTHORIZATION_NAME_SIZE] = {0};
BOOL32 g_UsernamePass = FALSE;
BOOL32 gOspexit = FALSE;

#ifdef _LINUX_
TCmdTable g_tInnerCmdTable[CMD_TABLE_SIZE];
int	 g_dwInnerCmdIndex = 0;
TCmdTable g_tCmdTable[RegCMD_TABLE_SIZE];
int	 g_iCmdIndex = 0;
#elif defined (_MSC_VER)
/* shell��̾�� */
HANDLE hShellProc = NULL;
HANDLE hShellThread = NULL;
/* ģ�鶯̬ע��� */
HMODULE ahRegModule[MAX_MOD_NUM]; 
/* ��֪ģ��� */
char *pachModTable[MAX_MOD_NUM]={"OspDll.dll"};
#endif

TCmdHistory g_tCmdHistory;

UniformFuncCMD g_UniformFuncCMD = NULL;
void *g_UniformFuncCMDPrivate = NULL;


enum tel_state { tel_normal = 0, tel_nego = 1, tel_sub = 2 };
static int32_t seen_iac = 0;
static enum tel_state state;
static int32_t count_after_sb = 0;
/*====================================================================
函数名：OspPrintf
功能：把相应的内容显示到屏幕,存储到文件,不能屏蔽
算法实现：（可选项）
引用全局变量：
输入参数说明：
bScreen: 是否输出到屏幕,
bFile: 是否输出到文件,
szFormat: 格式,
返回值说明：
====================================================================*/
API void OspExit(){
	gOspexit = TRUE;
}
API void OspUniformFuncRedirect(UniformFuncCMD fun,void *private){
	g_UniformFuncCMD = fun;
	g_UniformFuncCMDPrivate = private;
}
API void OspPrintf(BOOL32 bScreen,BOOL32 bFile, char * szFormat,...)
{
//     TOspLogHead tOspLogHead;
    char msg[MAX_LOG_MSG_LEN] = {0};
    uint32_t actLen = 0;
    va_list pvList;

    if(szFormat == NULL)
    {
        return;
    }
//     tOspLogHead.m_byType = LOG_TYPE_UNMASKABLE;
//     tOspLogHead.m_bToScreen = bScreen;
//     tOspLogHead.m_bToFile = bFile;

    va_start(pvList, szFormat);
    actLen = vsprintf(msg, szFormat, pvList);
    va_end(pvList);
    if(actLen <= 0 || actLen >= MAX_LOG_MSG_LEN)
    {
        printf("Osp: vsprintf() failed in COspLog::LogQuePrint().\n");
        return;
    }
//     LogQueWrite(&g_tOsp.m_cLogTask,tOspLogHead, msg, actLen);

    //to do..
    TelePrint(msg);
}
static void osphelp()
{
	char buff[100] ={0};
	sprintf(buff,"%s\n","Osp Help:");
	TelePrint(buff);
	OspRegCmdShow();
}
static void ospver()
{
	TelePrint("Osp [1.0.0]\n");
}
static void SendIAC(char cmd, char opt)
{
    uint8_t buf[5];
    buf[0] = TELCMD_IAC;
    buf[1] = cmd;
    buf[2] = opt;
/*	telSend((char*)buf, 3);*/
    TeleCmdEcho((char*)buf, 3);
}

static char remove_iac(uint8_t c)
{
    char ret = 0;
    if ((c == 255) && !seen_iac)    /* IAC*/
    {
        seen_iac = 1;
        return ret;
    }
    
    if (seen_iac)
    {
        switch(c)
        {
        case 251:
        case 252:
        case 253:
        case 254:
            if (state != tel_normal) {
                printf(" illegal negotiation.\n"); 
            }
            state = tel_nego;
            break;
        case 250:
            if (state != tel_normal){
                printf(" illegal sub negotiation.\n"); 
            }
            state = tel_sub;
            count_after_sb = 0;
            break;
        case 240:
            if (state != tel_sub) {
                printf(" illegal sub end.\n"); 
            }
            state = tel_normal;
            break;
        default:
            if (!((c > 240) && (c < 250) && (state == tel_normal)))
            {
                printf("illegal command.\n"); 
            }
            state = tel_normal;
        }
        seen_iac = 0;
        return 0;
    }
    
    switch (state)
    {
    case tel_nego:
        state = tel_normal;
        break;
    case tel_sub:
        count_after_sb++; /* set maximize sub negotiation length*/
        if (count_after_sb >= 100) state = tel_normal;
        break;
    default:
        ret = c;
    }
    return ret;
}

#ifdef _LINUX_
static UniformFunc FindCommand(const char* name)
{
   int i = 0;
    for(i = 0; i < g_dwInnerCmdIndex; i++)
    {
	    if (strcmp(g_tInnerCmdTable[i].name, name) == 0)
        {
	        return g_tInnerCmdTable[i].cmd;
        }
    }

    for(i = 0; i < g_iCmdIndex; i++)
    {
	if (strcmp(g_tCmdTable[i].name, name) == 0)
		{
		    return g_tCmdTable[i].cmd;
		}
    }
    return NULL;
}
#endif

void FindHistoryCommand(char *pCommand, uint16_t mode)
{
	int i;
	int cmdlen;

	if(NULL == pCommand)
	{
		return;
	}

	cmdlen = strlen(pCommand);

	if ((cmdlen-1)*3 + 2 > MAX_COMMAND_LENGTH)
	{
		OspPrintf(1,0,"[FindHistoryCommand] cmd len:%d\n", cmdlen);
		return ;
	}

	switch(mode)
	{
		case 1:
		{
			if(0 ==g_tCmdHistory.m_wNum)
			{
				return;
			}
			
			if((0 ==g_tCmdHistory.m_wFindIdx) && (g_tCmdHistory.m_wNum < CMD_TABLE_SIZE))
			{
				return;
			}
			g_tCmdHistory.m_wFindIdx = (g_tCmdHistory.m_wFindIdx + CMD_TABLE_SIZE -1)%CMD_TABLE_SIZE;	

			if (0 == cmdlen)
			{
				strcpy(pCommand, g_tCmdHistory.m_abyCmdStr[g_tCmdHistory.m_wFindIdx]);
				TeleCmdEcho(pCommand, strlen(pCommand));
			}
			else if (strcmp(pCommand, g_tCmdHistory.m_abyCmdStr[g_tCmdHistory.m_wFindIdx]))
			{
				for(i = 0; i< cmdlen; i++)
				{
					pCommand[3*i] = BACKSPACE_CHAR;
					pCommand[3*i+1] = BLANK_CHAR;
					pCommand[3*i+2] = BACKSPACE_CHAR;
				}
				TeleCmdEcho(pCommand, 3*i);
				strcpy(pCommand, g_tCmdHistory.m_abyCmdStr[g_tCmdHistory.m_wFindIdx]);
				TeleCmdEcho(pCommand, strlen(pCommand));
			}
			
			break;
		}
		case 2: 
		{
			if (g_tCmdHistory.m_wFindIdx == (g_tCmdHistory.m_wCurIdx + CMD_TABLE_SIZE - 1)%CMD_TABLE_SIZE)
			{
				if(cmdlen > 0)
				{
					for(i = 0; i< cmdlen; i++)
					{
						pCommand[3*i] = BACKSPACE_CHAR;
						pCommand[3*i+1] = BLANK_CHAR;
						pCommand[3*i+2] = BACKSPACE_CHAR;
					}
					TeleCmdEcho(pCommand, 3*i);
				}
				g_tCmdHistory.m_wFindIdx = (g_tCmdHistory.m_wFindIdx + 1)%CMD_TABLE_SIZE;
				memset(pCommand, 0, MAX_COMMAND_LENGTH);
			}
			else if (g_tCmdHistory.m_wFindIdx < (g_tCmdHistory.m_wCurIdx + CMD_TABLE_SIZE)%CMD_TABLE_SIZE)
			{
				if(cmdlen > 0)
				{
					for(i = 0; i< cmdlen; i++)
					{
						pCommand[3*i] = BACKSPACE_CHAR;
						pCommand[3*i+1] = BLANK_CHAR;
						pCommand[3*i+2] = BACKSPACE_CHAR;
					}
					TeleCmdEcho(pCommand, 3*i);
				}
				g_tCmdHistory.m_wFindIdx = (g_tCmdHistory.m_wFindIdx + 1)%CMD_TABLE_SIZE;
				strcpy(pCommand, g_tCmdHistory.m_abyCmdStr[g_tCmdHistory.m_wFindIdx]);
				TeleCmdEcho(pCommand, strlen(pCommand));
			}
			break;
		}
		case 5:
		{
#ifdef _LINUX_

			uint16_t wRegFindNum = 0;
			uint16_t wInnerFindNum = 0;
			uint16_t awRegFindIdx[512];
			uint16_t awInnerFindIdx[256];
			char  abyTmpCmd[1000];
			uint16_t wTmpCmdSize = 0;

			memset(abyTmpCmd, 0, 1000);

			for(i = 0; i < g_iCmdIndex; i++)
			{
				if(!(strstr(g_tCmdTable[i].name, pCommand)-g_tCmdTable[i].name))
				{
					awRegFindIdx[wRegFindNum] = i;
					wRegFindNum++;
				}
			}
			for(i = 0; i < g_dwInnerCmdIndex; i++)
			{
				if(!(strstr(g_tInnerCmdTable[i].name, pCommand)-g_tInnerCmdTable[i].name))
				{
					awInnerFindIdx[wInnerFindNum] = i;
					wInnerFindNum++;
				}
			}
			if((wRegFindNum+ wInnerFindNum) > 1)
			{
				if(wRegFindNum > 0)
				{
					for(i = 0; i < wRegFindNum; i++)
					{
						if (wTmpCmdSize >= 963)
						{
							break;
						}			
						strcat(abyTmpCmd, g_tCmdTable[awRegFindIdx[i]].name);
						strcat(abyTmpCmd, " 	");
						wTmpCmdSize = wTmpCmdSize + sizeof(g_tCmdTable[awRegFindIdx[i]].name);
						wTmpCmdSize = wTmpCmdSize + 4;
					}
				}
				if(wInnerFindNum > 0)
				{
					for(i = 0; i < wInnerFindNum; i++)
					{
						if (wTmpCmdSize >= 963)
						{
							break;
						}
						strcat(abyTmpCmd, g_tInnerCmdTable[awInnerFindIdx[i]].name);
						strcat(abyTmpCmd, " 	");
						wTmpCmdSize = wTmpCmdSize + sizeof(g_tInnerCmdTable[awInnerFindIdx[i]].name);
						wTmpCmdSize = wTmpCmdSize + 4;
					}
				}
				TeleCmdEcho("\r\n", 2); 	
				TeleCmdEcho(abyTmpCmd, strlen(abyTmpCmd));
				TeleCmdEcho("\r\n", 2); 
				PromptShow();		  // ��ʾ��ʾ��
				TeleCmdEcho(pCommand, cmdlen);
			}
			else if (1 == wRegFindNum)
			{
				for(i = 0; i< cmdlen; i++)
				{
					abyTmpCmd[3*i] = BACKSPACE_CHAR;
					abyTmpCmd[3*i+1] = BLANK_CHAR;
					abyTmpCmd[3*i+2] = BACKSPACE_CHAR;
				}
				strcpy(pCommand, g_tCmdTable[awRegFindIdx[0]].name);
				if (wTmpCmdSize >= 963)
				{
					break;
				}
				strcat(abyTmpCmd, g_tCmdTable[awRegFindIdx[0]].name);
				wTmpCmdSize = wTmpCmdSize + sizeof(g_tCmdTable[awRegFindIdx[0]].name);
				TeleCmdEcho(abyTmpCmd, strlen(abyTmpCmd));
			}
			else if (1 == wInnerFindNum)
			{
				for(i = 0; i< cmdlen; i++)
				{
					abyTmpCmd[3*i] = BACKSPACE_CHAR;
					abyTmpCmd[3*i+1] = BLANK_CHAR;
					abyTmpCmd[3*i+2] = BACKSPACE_CHAR;
				}
				strcpy(pCommand, g_tInnerCmdTable[awInnerFindIdx[0]].name);
				if (wTmpCmdSize >= 963)
				{
					break;
				}
				strcat(abyTmpCmd, g_tInnerCmdTable[awInnerFindIdx[0]].name);
				wTmpCmdSize = wTmpCmdSize + sizeof(g_tInnerCmdTable[awInnerFindIdx[0]].name);
				TeleCmdEcho(abyTmpCmd, strlen(abyTmpCmd));
			}

			break;
#endif			
		}
	}

	return;
}

/*====================================================================
  ������OspTeleDaemon
  ���ܣ�Telnet�������ⲿ������������
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����sockClient: ��Telnet�ͻ����ӵ��׽���,
  g_nSockTelSer: �����׽���,
  g_nSockTelClientOld: ǰһ����Telnet�ͻ����ӵ��׽���.
  �������˵����uPort: ����˿ںţ�
            
  ����ֵ˵������.
  ====================================================================*/

API void* OspTeleDaemon(uint16_t pwPort, const char * name)
{
#ifdef _LINUX_
#ifndef _IOS_
	{
		char pname[128];
		//OspRegTaskInfo( OspTaskSelfID() , "OspTeleDaemon" );
		
		sprintf(pname, "%s", name == NULL ? "telesrv":name);

		prctl(15, pname, 0, 0, 0);

	}
#endif
#endif

    //mod by gzj. 081103. �忨��֧��epoll. �ָ���epoll ��ʽ.

    struct sockaddr_in addrClient;

    uint16_t port = 0 ;  
    uint16_t wPort = pwPort;
    socklen addrLenIn = sizeof(addrClient);

	char cmdChar = 0;
	char getCmd[4];
    char command[MAX_COMMAND_LENGTH] = {0};
    uint8_t cmdLen = 0;

    g_nSockTelSer = INVALID_SOCKET; 
    g_wportListtening = 0;

    /* ���ָ���˿ںţ���ΪTelnet�������ڸö˿ں��ϴ���һ���׽��� */
    if(wPort != 0)
    {
		g_nSockTelSer = CreateTcpNodeNoRegist(0, wPort, TRUE); // server's port
		if(g_nSockTelSer != INVALID_SOCKET) 
		{
			g_wportListtening = wPort;			
		}
	}

    /* ���δָ���˿ںŻ���ָ���˿ں��ϴ����׽���ʧ�ܣ���OSP���д��� */
    if(g_nSockTelSer == INVALID_SOCKET)
    {
		for(port=MIN_TELSVR_PORT; port<MAX_TELSVR_PORT; port++)
		{
		    g_nSockTelSer = CreateTcpNodeNoRegist(0, port, FALSE); // server's port
		    if(g_nSockTelSer != INVALID_SOCKET) 
		    {
				g_wportListtening = port;
				break;
		    }			
		}
    }
    
    if(g_nSockTelSer == INVALID_SOCKET)
    {
#ifdef _MSC_VER
		/* ���OSP���ܴ���Telnet�����׽��֣���������� */
		OspTaskSuspend((TASKHANDLE)OspTaskSelfID());
#else
		// OspTaskSuspend was not implemented under Linux
		//OspTaskExit();
#endif
    }

	/* listen ��socketӦ������Ϊ�������ֹaccept��ʱ���ܵ�����*/
	OSPSetSockNoBlock(g_nSockTelSer);
	    
    /* ����û�������OspQuit, �˳������� */
    while(!gOspexit)
    {       
		int ret;
		fd_set m_fdSet;
		SOCKHANDLE maxSocket;

		memset(getCmd, 0, 4);
		FD_ZERO(&m_fdSet);
		FD_SET(g_nSockTelSer, &m_fdSet);	
		
		if (g_nSockTelClient != INVALID_SOCKET)
		{
			FD_SET(g_nSockTelClient, &m_fdSet);
		}
		
		if (g_nSockTelSer > g_nSockTelClient)
		{
			maxSocket = g_nSockTelSer + 1;
		}
		else
		{
			maxSocket = g_nSockTelClient +1;
		}
		
		ret = select(maxSocket, &m_fdSet, NULL, NULL, NULL);

		if (ret == SOCKET_ERROR)
		{
			printf("TeleDaemon : Telnet Server Select Error %d!!! \r\n",OSPGetSockError());
			OspDelay(500);
			continue;
		}

		if (FD_ISSET(g_nSockTelSer, &m_fdSet))
		{
            uint32_t dwOn = 1;
			SOCKHANDLE m_NewTelHandle = accept(g_nSockTelSer, (struct sockaddr *)&addrClient, &addrLenIn);

            if (ioctl(m_NewTelHandle, FIONBIO, &dwOn) < 0)
            {
                printf("cannot set telnet client socket no block, socket:%d\n", m_NewTelHandle);
            }

			if (m_NewTelHandle == INVALID_SOCKET)
			{
				printf("TeleDaemon : Telnet Server Accept Error %d!!! \r\n",OSPGetSockError());
				continue;
			}
			if (g_nSockTelClient!= INVALID_SOCKET)
			{
				OspSockClose( g_nSockTelClient);
			}
			g_nSockTelClient = m_NewTelHandle;
			
			//add by gzj. 080908. ���֮ǰ��cmd����.
			memset(command,0,sizeof(command));
			cmdLen = 0;
			
			
			/* ����TELE���ԣ���ӡ��ӭ���*/
			SendIAC(TELCMD_DO, TELOPT_ECHO);
			SendIAC(TELCMD_DO, TELOPT_NAWS);
			SendIAC(TELCMD_DO, TELOPT_LFLOW);
			SendIAC(TELCMD_WILL, TELOPT_ECHO);
			SendIAC(TELCMD_WILL, TELOPT_SGA);
			
			/* �����ӭ���� */
			TelePrint("*===============================================================\n");
			TelePrint("*\t\t Welcome to KEDACOM telnet.\n");
			TelePrint("*===============================================================\n");
									
			/* �����ʾ�� */
			g_PromtState = PROMTUSER;
			PromptShow();

			continue;
		}

		if (FD_ISSET(g_nSockTelClient, &m_fdSet))
		{
			//����ؽ����û�����
			ret = recv(g_nSockTelClient, getCmd, 3, 0);
			
			//�ͻ��˹ر�
			if(ret == 0)
			{
			//	printf("OspTeleDaemon: peer closed\n");
				OspSockClose(g_nSockTelClient);
				g_nSockTelClient = INVALID_SOCKET;
				continue;
			}
			
			//���˹ر�
			if(ret == SOCKET_ERROR)
			{
				printf("OspTeleDaemon: recv error %d\n",OSPGetSockError());
				OspSockClose(g_nSockTelClient);
				g_nSockTelClient = INVALID_SOCKET;
				OspDelay(500);
				continue;
			}		
			
			cmdChar = getCmd[0];
	#ifdef _MSC_VER
			if((-1 == cmdChar) && (3 == ret))
			{
				uint8_t byIdx = 0;
				for(byIdx = 0; byIdx < ret; byIdx++)
				{
					cmdChar = getCmd[byIdx];
					cmdChar = remove_iac(cmdChar);
				}
			}
			else
			{
				cmdChar = remove_iac(cmdChar);
			}
	#else	
			if((255 == cmdChar) && (3 == ret))
			{
				uint8_t byIdx = 0;
				for(byIdx = 0; byIdx < ret; byIdx++)
				{
					cmdChar = getCmd[byIdx];
			cmdChar = remove_iac(cmdChar);
				}
			}
			else
			{
				cmdChar = remove_iac(cmdChar);
			}
	#endif	
			//�����û�����, ���Ե�Telnet�ͻ�����Ļ��, �����������ʵ�����Ӧ
			switch(cmdChar)
			{
				case CTRL_S:
					//OspStopScrnLog();
					OspPrintf(TRUE, FALSE, "\nScreen log disabled temporarily. Use ^R to resume it if necessary.\n\n");
					PromptShow();
					break;
				
				case CTRL_R:
					//OspResumeScrnLog();
					OspPrintf(TRUE, FALSE, "\nScreen log resumed again.\n");
					PromptShow();
					break;
				
				case RETURN_CHAR:		  // �س���
					TeleCmdEcho("\r\n", 2); 		   
					if((cmdLen != 0) && (g_PromtState == PROMTAUTHORIZED))
					{   
                        uint16_t tmp_wNum = 0; 
						strcpy(g_tCmdHistory.m_abyCmdStr[g_tCmdHistory.m_wCurIdx], command);
						g_tCmdHistory.m_wCurIdx = (g_tCmdHistory.m_wCurIdx + 1)%CMD_TABLE_SIZE;
						g_tCmdHistory.m_wFindIdx = g_tCmdHistory.m_wCurIdx;
                        tmp_wNum = ++g_tCmdHistory.m_wNum;
						g_tCmdHistory.m_wNum = (g_tCmdHistory.m_wNum == CMD_TABLE_SIZE) ? CMD_TABLE_SIZE : tmp_wNum;
					}
					
					CmdParse(command, cmdLen);
				
					cmdLen = 0; 		  
					memset(command,0,MAX_COMMAND_LENGTH);
					PromptShow();		  // ��ʾ��ʾ��
					break;
				
				case NEWLINE_CHAR:
					break;
					
				case ARROW_Start:			  // �ϼ�ͷ
				{
					if (3 == ret)
					{						
						if (!strcmp("\033[A", getCmd))
						{
							FindHistoryCommand(command, 1);
							cmdLen = strlen(command);
						}
						else if (!strcmp("\033[B", getCmd))
						{
							FindHistoryCommand(command, 2);
							cmdLen = strlen(command);
						}
						else if (!strcmp("\033[C", getCmd))
						{
							//printf("right arrow\n");
						}
						else if (!strcmp("\033[D", getCmd))
						{
							//printf("left arrow\n");
						}
					}
					
					break;
				}
				case TAB_CHAR:
				{
					FindHistoryCommand(command, 5);
					cmdLen = strlen(command);
					break;
				}
				case BACKSPACE_CHAR:		 // �˸��
				{
					if(cmdLen <= 0)
					{
						continue;
					}
					
					cmdLen--;	
					if(cmdLen < MAX_COMMAND_LENGTH)
					{			 
						command[cmdLen] = '\0';
					}
					else
					{
						OspPrintf(TRUE, FALSE, "err cmdLen:%d\n", cmdLen);
						break;
					}
					if(g_PromtState != PROMTPWD)
					{			 
						/* ʹ�����ˣ���һ���ո����ԭ�ַ���ʹ������ */
						char tmpChar[3];
				
						tmpChar[0] = BACKSPACE_CHAR;
						tmpChar[1] = BLANK_CHAR;
						tmpChar[2] = BACKSPACE_CHAR;
						TeleCmdEcho(tmpChar, 3);
					}
					break;
				}
				default:
					if(g_PromtState != PROMTPWD)
					{ 
						TeleCmdEcho(&cmdChar, 1);
					}
					if(cmdLen < MAX_COMMAND_LENGTH-1)
					{				
						command[cmdLen++] = cmdChar;
						command[cmdLen]='\0';
					}
					else
					{				 
						OspPrintf(TRUE, FALSE, "\n");
						CmdParse(command, cmdLen);	
				
						PromptShow();		  // ��ʾ��ʾ��
						cmdLen = 0;
						memset(command,0,MAX_COMMAND_LENGTH);
					}
					break;
			}
		}
	} 
    if(g_nSockTelClient != INVALID_SOCKET)
    {
    	OspSockClose(g_nSockTelClient);
    	g_nSockTelClient = INVALID_SOCKET;
    }
    //OspTaskExit();
	
    return NULL;                // ������澯
}

/*====================================================================
������CheckAuthorization
���ܣ�����û�����Ȩ,������Ӧ״̬����.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����szCmd: �յ��������ַ�,dwCmdlen:�ַ�ĳ���.
            
����ֵ˵������.
====================================================================*/
API void CheckAuthorization(char *szCmd,uint32_t dwCmdlen)
{
    char szInput[AUTHORIZATION_NAME_SIZE];
    if((dwCmdlen >=AUTHORIZATION_NAME_SIZE)&&(g_PromtState != PROMTAUTHORIZED))
    {
        OspPrintf(TRUE,FALSE,"Osp:CMD NAME is too long! dwCmdlen=%d\n",dwCmdlen);
        return;
    }  
    if(dwCmdlen>0)
    {    
        switch(g_PromtState)
        {    
        case PROMTUSER:
            {
                strncpy(szInput,szCmd,dwCmdlen);
                szInput[dwCmdlen]='\0';
                if(strcmp(g_TelnetUsername,szInput) == 0)
                {
                      g_UsernamePass = TRUE;
                }
                else
                {
                    g_UsernamePass = FALSE;
                }
                g_PromtState = PROMTPWD;  
                break;
            }
        case PROMTPWD:
            {
                strncpy(szInput,szCmd,dwCmdlen);
                szInput[dwCmdlen]='\0';                
                if(strcmp(g_TelnetPasswd,szInput) == 0)
                {
                    if(g_UsernamePass == TRUE)
                    {                
                        g_PromtState = PROMTAUTHORIZED;
                    }
                    else
                    {                    
                        g_PromtState = PROMTUSER;
                    }
                }
                else
                {
                    g_UsernamePass = FALSE;
                    g_PromtState = PROMTUSER;
                }
                break;
            }
        default:
            break;
        }
    }
    else
    {
        switch(g_PromtState)
        {
        case PROMTUSER:
            {
                if(strcmp(g_TelnetUsername,"") == 0)
                {
                      g_UsernamePass = TRUE;
                }
                else
                {
                    g_UsernamePass = FALSE;
                }
                g_PromtState = PROMTPWD;  
                break;
            }
        case PROMTPWD:
            {
                if(strcmp(g_TelnetPasswd,"") == 0)
                {
                    if(g_UsernamePass == TRUE)
                    {                
                        g_PromtState = PROMTAUTHORIZED;
                    }
                    else
                    {                    
                        g_PromtState = PROMTUSER;
                    }
                }
                else
                {
                    g_UsernamePass = FALSE;
                    g_PromtState = PROMTUSER;
                }
                break;
            } 
        default:
            break;
        }
    }

}

/*====================================================================
������OspTelAuthor
���ܣ�����Telnet����Ȩ�û��������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����
����ֵ˵����
====================================================================*/
API void OspTelAuthor(const char * szUsername ,const char * szPassword)
{
    if(szUsername != NULL)
    {
        uint32_t dwUsernameLen = strlen(szUsername);
        if(dwUsernameLen >= AUTHORIZATION_NAME_SIZE)
        {
            OspPrintf(TRUE,FALSE,"Osp: telnet username is too long!\n");
            return;
        }  
    }
    if(szPassword != NULL)
    {
        uint32_t dwPasswordLen = strlen(szPassword);
        if(dwPasswordLen >= AUTHORIZATION_NAME_SIZE)
        {
            OspPrintf(TRUE,FALSE,"Osp: telnet password is too long!\n");
            return;
        }  
    }

    if(szUsername == NULL)
    {
        strcpy(g_TelnetUsername,"");
    }
    else
    {
        strcpy(g_TelnetUsername,szUsername);
    }
    if(szPassword == NULL)
    {
        strcpy(g_TelnetPasswd,"");    
    }
    else
    {    
        strcpy(g_TelnetPasswd,szPassword);    
    }
}

/*====================================================================
  ������RunCmd
  ���ܣ������û�����, ����ʵ�����Ӧ.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����szCmd: �յ��������ַ�.
            
  ����ֵ˵������.
  ====================================================================*/
API void RunCmd(char *szCmd)
{ 
    long para[10];
    const char *ppara[10]; 
    TRawPara atRawPara[10];
    int paraNum = 0;
    int cmdArgs = 0;
    uint8_t count = 0;
    uint8_t chStartCnt = 0;
    BOOL32 bStrStart = FALSE;
    BOOL32 bCharStart = FALSE;
    uint32_t cmdLen = strlen(szCmd)+1;
	int ret = 0;
	UniformFunc cmdFunc;
#ifdef _MSC_VER
	HMODULE hModule;
	int i = 0;
#endif

    memset(para, 0, sizeof(para));
    memset(ppara, 0, sizeof(ppara));
    memset(atRawPara, 0, sizeof(TRawPara)*10);
    switch(g_PromtState)
    {
	    case PROMTUSER:
	    case PROMTPWD:
	        {
	            CheckAuthorization(szCmd,strlen(szCmd));
	            break;
	        }
	    case PROMTAUTHORIZED:
	        {
	            /* ������������ */
	            while( count < cmdLen )
	            {	
		        switch(szCmd[count])
		        {
		        case '\'':
		            szCmd[count] = '\0';
		            if(!bCharStart)
		            {
			        chStartCnt = count;
		            }
		            else
		            {
			        if(count > chStartCnt+2)
			        {
			            OspPrintf(TRUE, FALSE, "input error.\n");
			            return;
			        }
		            }
		            bCharStart = !bCharStart;
		            break;

		        case '\"':
		            szCmd[count] = '\0';
		            bStrStart = !bStrStart;
		            break;

		        case ',':
		        case ' ':
		        case '\t':
		        case '\n':
		        case '(':
		        case ')':
		            if( ! bStrStart )
		            {
			        szCmd[count] = '\0';
		            }
		            break;
				case '\0':
					break;
		        default:
		            /* ����ַ�Ϊ��Ч�ַ�ǰһ�ַ�ΪNULL����ʾ�ɵ��ʽ���
		               �µ��ʿ�ʼ */
		            if(count > 0 && szCmd[count-1] == '\0')
		            {				
                        atRawPara[paraNum].paraStr = &szCmd[count];
                        cmdArgs++;
                        
			        if(bStrStart)
			        {
			            atRawPara[paraNum].bInQuote = TRUE;
			        }
			        if(bCharStart)
			        {
			            atRawPara[paraNum].bIsChar = TRUE;
			        }
			        if(++paraNum >= 10)
			            break;
		            }
		        }
		        count++;
	        }

	        if(bStrStart || bCharStart)
	        {
		        OspPrintf(TRUE, FALSE, "input error.\n");
		        return;
	        }

	        for(count=0; count<10; count++)
	        {
		        if(atRawPara[count].paraStr == NULL)
		        {
		            para[count] = 0;
                    ppara[count] = NULL;
		            continue;
		        }

		        if(atRawPara[count].bInQuote)
		        {
		            para[count] = (long)atRawPara[count].paraStr;
                    ppara[count] = atRawPara[count].paraStr;
		            continue;
		        }

		        if(atRawPara[count].bIsChar)
		        {
		            para[count] = (char)atRawPara[count].paraStr[0];
                    ppara[count] = atRawPara[count].paraStr;
		            continue;
		        }

		        para[count] = WordParse(atRawPara[count].paraStr);
                ppara[count] = atRawPara[count].paraStr;
	        }

	        /* ��ִ������ */
	        if ( strcmp("bye", szCmd) == 0 )
	        {
		        OspPrintf(TRUE, FALSE, "\n  bye......\n");
		        OspDelay(500);            // not reliable,
		        OspSockClose(g_nSockTelClient);
		        g_nSockTelClient = INVALID_SOCKET;
		        return;
	        }    

	        if (strcmp("osphelp", szCmd) == 0)
	        {
		        osphelp();
		        return;
	        } 

		if (strcmp("ospver", szCmd) == 0)
	        {
		        ospver();
	  		    return;
	        } 
		
			
#ifdef _LINUX_

	        cmdFunc = FindCommand(szCmd);
			UniformFuncCMD _gfuncmd = g_UniformFuncCMD;
			if (_gfuncmd){
					 ret = _gfuncmd(szCmd,ppara,cmdArgs,g_UniformFuncCMDPrivate);
				}
            else if (cmdFunc != NULL)
	        {
	            ret = (*cmdFunc)(para[0],para[1],para[2],para[3],para[4],para[5],para[6],para[7],para[8],para[9]);
	            OspPrintf(TRUE, FALSE, "Return value: %d\n", ret);
	        } 
            else
	        {
	            OspPrintf(TRUE, FALSE, "function '%s' doesn't exist!\n", szCmd);
	        }
	        break;
			
#elif defined (_MSC_VER)
				
			/* �鿴�Ƿ�ǰģ���еĺ��� */
			hModule = GetModuleHandle(NULL);  // ����NULL��ʾ��ǰģ��
			if(hModule != NULL)    
			{
				UniformFuncCMD _gfuncmd = g_UniformFuncCMD;
				cmdFunc = (int (* )(int,int,int,int,int,int,int,int,int,int)) GetProcAddress(hModule, szCmd); 
				if (_gfuncmd){
						 ret = _gfuncmd(szCmd,para,g_UniformFuncCMDPrivate);
					}
				if(cmdFunc != NULL)
				{
					ret = cmdFunc(para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7], para[8], para[9]);
				}
					OspPrintf(TRUE, FALSE, "\nvalue=%d\n", ret);
					return;
			}
			
			/* �ٲ鿴�Ƿ�Ǽ�ģ���еĺ��� */
			for(i=0; i<MAX_MOD_NUM; i++)
			{
				if( (hModule = ahRegModule[i]) != NULL &&
					(cmdFunc = (int (* )(int,int,int,int,int,int,int,int,int,int))
										 GetProcAddress(hModule, szCmd)) != NULL )
				{
					ret = cmdFunc(para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7], para[8], para[9]);
					OspPrintf(TRUE, FALSE, "\nvalue=%d\n", ret);
					return;
				}
			}
			
			/* �ٲ��Ƿ���֪ģ���еĺ��� */	
			for(i=0; i<MAX_MOD_NUM; i++)
			{
				if( pachModTable[i] != NULL &&
					(hModule = GetModuleHandle(pachModTable[i])) != NULL )
				{
					if( (cmdFunc = (int (* )(int,int,int,int,int,int,int,int,int,int))
								   GetProcAddress(hModule, szCmd)) != NULL )
					{	
						ret = cmdFunc(para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7], para[8], para[9]);
						OspPrintf(TRUE, FALSE, "\nvalue=%d\n", ret);
						return; 			 
					}
				}
			} 
			
			OspPrintf(TRUE, FALSE, "function '%s' doesn't exist!\n", szCmd);
			break;
	#endif
	
        }
    }
    return;
}

/*====================================================================
  ������CmdParse
  ���ܣ������׼���ȥ����ͷ����Ч�ַ�������'\0'.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����pchCmd: ���, 
  uCmdLen: �����.
            
  ����ֵ˵������.
  ====================================================================*/
API void CmdParse(LPCSTR pchCmd, uint8_t byCmdLen)
{  
    uint8_t count;
    int nCpyLen = 0;
    char command[MAX_COMMAND_LENGTH];

    if(byCmdLen > 0)
    {       
        //ȥͷ
        for(count=0; count<byCmdLen; count++)
        {
		    char chTmp;

		    chTmp = pchCmd[count];

		    if (isdigit(chTmp) || islower(chTmp) || isupper(chTmp))
		    {
		        break;
		    }
        }

        nCpyLen = byCmdLen-count;
    }
    
    if(nCpyLen <= 0)
    {
        CheckAuthorization(command,0);  
        return;
    }

    memcpy(command, pchCmd+count, nCpyLen);   
    if(byCmdLen < MAX_COMMAND_LENGTH)
    {
		command[nCpyLen] = '\0';
    }
    else
    {
		command[MAX_COMMAND_LENGTH-1] = '\0';
    }

    RunCmd(command);
}

/*====================================================================
  ������WordParse
  ���ܣ�ת���������.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����word: �����������, 
                          
  ����ֵ˵����������Ϊ����, ���ظ�����; Ϊ��ͨ�ַ��򷵻ظ��ַ�ָ��.
  ====================================================================*/
API long WordParse(LPCSTR word)
{
    int tmp;
	uint16_t len = 0;
	char chTmp = 0;
	
    if(word == NULL) 
    {
		return 0;
    }
	
    tmp = atoi(word);
    if((tmp == 0) && word[0] != '0')
    {
		return (long)word;
    }
	
	chTmp = word[0];
	while(isdigit(chTmp))
	{
		len++;
		chTmp = word[len];
	}

	if(len == strlen(word))
	{
    	return tmp;
	}
	else
	{
		return (long)word;
	}
}

/*====================================================================
  ������TeleCmdEcho
  ���ܣ���Telnet�ͻ�����Ļ�����û�����.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����pchCmdStr: ���,
  uLen: �����. 
                          
  ����ֵ˵������.
  ====================================================================*/
API void TeleCmdEcho(LPCSTR pchCmdStr, uint32_t dwLen)
{
    char achCmd[512] = {0};
    if(dwLen < sizeof(achCmd))
    {
        memcpy(achCmd,pchCmdStr,dwLen);
        TelePrint(achCmd);
    }
    else
    {
        printf("[Osp]cmd too long!\n");
    }

}

/*====================================================================
  ������OspShellStart
  ���ܣ���Windows�´���һ�����������Telnet�ͻ���.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����g_wportListtening: Telnet������������˿�.
  �������˵����

  ����ֵ˵������.
  ====================================================================*/
API void OspShellStart()
{ 
#ifdef _MSC_VER
    char command[100];
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;	

    sprintf((char*)command, "telnet.exe localhost %d", g_wportListtening);

    memset(&siStartInfo, 0, sizeof(STARTUPINFO));
    memset(&piProcInfo, 0, sizeof(PROCESS_INFORMATION));

    siStartInfo.cb = sizeof(STARTUPINFO); 
    if( !CreateProcess(NULL, command, NULL, NULL, FALSE, 
		       CREATE_NEW_CONSOLE, NULL, NULL, &siStartInfo, &piProcInfo) )
    {
	OspLog(1, "Osp: create process for shell failed.\n");
	 
	return;
    }

    hShellProc = piProcInfo.hProcess;
    hShellThread = piProcInfo.hThread;
#endif
}

/*====================================================================
  ������OspShellExit
  ���ܣ���Windows���˳�Shell.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����hShellProc--Shell��̾��.
  �������˵����

  ����ֵ˵������.
  ====================================================================*/
API void OspShellExit()
{
#ifdef _MSC_VER
    unsigned long exitCode;

    if(hShellProc != NULL)
    {
	GetExitCodeProcess(hShellProc, &exitCode);
	if(exitCode == STILL_ACTIVE)
	{
	    TerminateProcess(hShellProc, 0);
	}
	CloseHandle(hShellProc);
	hShellProc = NULL;
    }

    if(hShellThread != NULL)
    {
	CloseHandle(hShellThread);
	hShellThread = NULL;
    }
#endif
}

/*====================================================================
  ������GetBaseModName
  ���ܣ���Windows�µõ�һ��ģ����ķ�·��������Ϊȱʡ����ʾ��.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����pchModName: ģ��ȫ��

  ����ֵ˵����ģ����Ļ���.
  ====================================================================*/
API char *GetBaseModName(char *pchModName)
{
    char *sep1 = "\\";
    char *sep2 = ".";
    char *pchBaseModName=NULL;
    char *token=NULL;

    token = strtok(pchModName, sep1);
    while( token != NULL )
    {
	pchBaseModName = token;
	token = strtok(NULL, sep1);	  
    }   

    pchBaseModName = strtok( pchBaseModName, sep2);
    return pchBaseModName;
}

/*====================================================================
  ������PromptShow
  ���ܣ���Telnet����ʾ��ʾ��.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵������

  ����ֵ˵������.
  ====================================================================*/
API void PromptShow(void)
{
    char achModName[MAX_MODNAME_LEN];
    char *pchBaseModName=NULL;
    char prompt[MAX_PROMPT_LEN + 4];	/* "%s->"*/
#ifdef _MSC_VER
	int ret = 0;
#endif

    switch(g_PromtState)
    {
    case PROMTUSER:
        {
            sprintf(prompt, "Username:");            
            TeleCmdEcho(prompt, strlen(prompt)+1);
            break;
        }
    case PROMTPWD:
        {          
            sprintf(prompt, "Password:");
            TeleCmdEcho(prompt, strlen(prompt)+1);
            break;
        }
    case PROMTAUTHORIZED:
        {
            /* ����û�ָ������ʾ��ʹ���� */
        	char shellname[] = "By.Chao";
            if(strlen(shellname) > 0)
            {
	        sprintf(prompt, "%s->", shellname);
	        TeleCmdEcho(prompt, strlen(prompt)+1);
	        return;
            }

            /* ȡ�õ�ǰ������ڵ�ģ������ΪTelnet�ͻ�����ʾ�� */
        #ifdef _MSC_VER
             ret = GetModuleFileName(NULL, achModName, MAX_MODNAME_LEN);			
            if(ret != 0)
            {
				achModName[MAX_MODNAME_LEN-1] = '\0';
				pchBaseModName = GetBaseModName(achModName);
            }
        #elif defined(_LINUX_)
            if (NULL != getcwd(achModName, sizeof(achModName)))
            {
				pchBaseModName = basename(achModName);
            }
        #endif
            if(pchBaseModName != NULL)
            {
				sprintf(prompt, "%s->", pchBaseModName);
            }
            else
            {
				sprintf(prompt, "\n");
            }

            TeleCmdEcho(prompt, strlen(prompt)+1);
            break;
        }
    default:
        {
            OspLog(1,"Osp Telnet Prompt State error!\n");
            break;
        }
    }	
}

/*====================================================================
  ������TelePrint
  ���ܣ���Telnet��Ļ�ϴ�ӡ��NULL��β���ַ�
  �㷨ʵ�֣����ַ����ʵ���ת������Ҫ�ǰ�'\n'ת��Ϊ'\r\n'��
  ���͵�Telnet�ͻ���.
  ����ȫ�ֱ�����
  �������˵����pchMsg: Ҫ��ӡ���ַ�

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
  ====================================================================*/
API BOOL32 TelePrint(LPCSTR pchMsg)
{
    char chCur;
    uint32_t dwStart = 0;
    uint32_t dwCount = 0;
    char *pchRetStr = "\n\r";
    BOOL32 bSendOK = FALSE;
	SOCKHANDLE m_localSock = g_nSockTelClient;

    if( (pchMsg == NULL) || (m_localSock == INVALID_SOCKET) )
    {
		return FALSE;
    }

    while(TRUE)
    {
		chCur = pchMsg[dwCount];

		/* ����'\n'��'\0', �����ǰһ��'\n'����'\n'֮��������ַ� */
		if(chCur == '\0' || chCur == '\n')
		{
		    bSendOK = SockSend(m_localSock, &pchMsg[dwStart], dwCount-dwStart);
		    if( !bSendOK )
		    {
				return FALSE;
		    }

		    /* ����'\n', ���"\r\n" */
		    if(chCur == '\n')
		    {
				bSendOK = SockSend(m_localSock, pchRetStr, 2);
				if( !bSendOK )
				{
				    return FALSE;
				}
		    }

		    /* ����'\0', ��ʾ�ַ����Ӧ���ѭ�� */
		    if(chCur == '\0')
		    {
				break;
		    }

		    /* ��һ��������� */
		    dwStart = dwCount+1;	
		}
		dwCount++;
    }
    return TRUE;
}

#ifdef _LINUX_
/*====================================================================
  ������OspRegInnerCommand
  ���ܣ�ע�������Telnet��ִ�е��ڲ�����
  �㷨ʵ�֣�
  ����ȫ�ֱ�����
  �������˵����
  	szName	- Telnet�����������
	pfFunc	- ��Ӧ�ĺ���ָ��
	szUsage	- ����İ�����Ϣ
  ����ֵ˵������
  ====================================================================*/
API void OspRegInnerCommand(const char* name, void* func, const char* usage)
{
    if (g_dwInnerCmdIndex < CMD_TABLE_SIZE)
    {
	strncpy(g_tInnerCmdTable[g_dwInnerCmdIndex].name, name, CMD_NAME_SIZE);
	strncpy(g_tInnerCmdTable[g_dwInnerCmdIndex].usage, usage, CMD_USAGE_SIZE);
	g_tInnerCmdTable[g_dwInnerCmdIndex].cmd = (UniformFunc)func;
	g_dwInnerCmdIndex++;
    }
}

/*====================================================================
  ������OspRegCommand
  ���ܣ�ע�������Telnet��ִ�е��ⲿ����
  �㷨ʵ�֣�
  ����ȫ�ֱ�����
  �������˵����
  	szName	- Telnet�����������
	pfFunc	- ��Ӧ�ĺ���ָ��
	szUsage	- ����İ�����Ϣ
  ����ֵ˵������
  ====================================================================*/
API void OspRegCommand(const char* name, void* func, const char* usage)
{
    if (g_iCmdIndex < RegCMD_TABLE_SIZE)
    {
	strncpy(g_tCmdTable[g_iCmdIndex].name, name, CMD_NAME_SIZE);
	strncpy(g_tCmdTable[g_iCmdIndex].usage, usage, CMD_USAGE_SIZE);
	g_tCmdTable[g_iCmdIndex].cmd = (UniformFunc)func;
	g_iCmdIndex++;
    }
}
API void OspRegCmdShow(void)
{
	uint16_t wIndex = 0;
	OspPrintf(TRUE, FALSE, "The total regcmd num is %d\n", g_iCmdIndex);
	for(wIndex = 0; wIndex < g_iCmdIndex; wIndex++)
	{
		OspPrintf(TRUE, FALSE, "The %d regcmd: %s; usage: %s\n",wIndex, g_tCmdTable[wIndex].name, g_tCmdTable[wIndex].usage);
	}
}

#endif

