#ifndef _OSP_TELE_H_
#define _OSP_TELE_H_

#include "ospcommon.h"

#define API
#ifdef __cplusplus
extern "C" {
#endif 

/* Telnet server's port range */
#define MIN_TELSVR_PORT  2500
#define MAX_TELSVR_PORT  8000/* 2800; for debug mulitnode */

#define MAX_MODNAME_LEN 255
#define MAX_MOD_NUM  20

#define MAX_COMMAND_LENGTH  (uint8_t)255
#define NEWLINE_CHAR  10
#define BACKSPACE_CHAR  8
#define BLANK_CHAR  32
#define RETURN_CHAR  13
#define TAB_CHAR  9
#define DEL_CHAR  127
#define CTRL_S 19
#define CTRL_R  18

#define ARROW_Start  27
#define UP_ARROW  24
#define DOWN_ARROW  25
#define LEFT_ARROW 26
#define RIGHT_ARROW  27



#define CMD_NAME_SIZE		32
#define CMD_TABLE_SIZE		256
// UniformFunc: definition for all functions those can invoked by user through telnet
typedef int (*UniformFunc)(int,int,int,int,int,int,int,int,int,int);
typedef int (*UniformFuncCMD)(const char *cmd,const char *args[],int size ,void *priv);

#ifdef _LINUX_

#define TELE_FD_NUM 3  //for epoll
#define CMD_USAGE_SIZE		64
#define RegCMD_TABLE_SIZE		512

typedef struct tagTCmdTable {
    char	name[CMD_NAME_SIZE];	/* Command Name; less than 30 bytes */
    UniformFunc cmd;			/* Implementation function */
    char	usage[CMD_USAGE_SIZE];	/* Usage message */
}TCmdTable;

#endif

typedef struct tagTCmdHistory
{
	char m_abyCmdStr[CMD_TABLE_SIZE][MAX_COMMAND_LENGTH]; 
	uint16_t m_wNum;
	uint16_t m_wCurIdx;
	uint16_t m_wFindIdx;
}TCmdHistory;

typedef enum PROMTSTATE
{
    PROMTUSER	 = 1,
    PROMTPWD	 = 2,
    PROMTAUTHORIZED	 = 3,
}PROMTSTATE; 

typedef struct 
{
	char *paraStr;
	BOOL32 bInQuote;
	BOOL32 bIsChar;
}TRawPara;


#define TELCMD_WILL    (uint8_t)251
#define TELCMD_WONT    (uint8_t)252
#define TELCMD_DO      (uint8_t)253
#define TELCMD_DONT    (uint8_t)254
#define TELCMD_IAC     (uint8_t)255

#define AUTHORIZATION_NAME_SIZE 20

#define TELOPT_ECHO     (uint8_t)1
#define TELOPT_SGA      (uint8_t)3
#define TELOPT_LFLOW    (uint8_t)33
#define TELOPT_NAWS     (uint8_t)34

API void* OspTeleDaemon(uint16_t pwPort, const char *name);

API void OspConsole(void);
API void PromptShow(void);
API void TeleCmdEcho(const char *pchCmdStr, uint32_t wLen);
API void CmdParse(const char *pchCmd, uint8_t byCmdLen);
API char *GetBaseModName(char *pchModName);
API void RunCmd(char * szCmd);
API long WordParse(const char *word);
API BOOL32 TelePrint(const char *pchMsg);
API void CheckAuthorization(char *szCmd,uint32_t dwCmdlen);
API void OspRegCommand(const char* name, void* func, const char* usage);
API void OspShellStart(void);
API void OspShellExit(void);
API void OspTelAuthor(const char * szUsername ,const char * szPassword);
API void OspUniformFuncRedirect(UniformFuncCMD fun, void *priv);
API void OspPrintf(BOOL32 bScreen,BOOL32 bFile, char * szFormat,...);
/*void* TeleSockEcho(void);*/
API void OspExit();

API void OspRegCmdShow(void);

API void OspDebug(uint32_t dwLevel);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif //OSP_TELE_INC

