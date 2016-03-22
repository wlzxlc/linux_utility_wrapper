#include "ospsock.h"
#include <unistd.h>
#include <ccstone/telnetd/osptele.h>
#include <errno.h>
SOCKHANDLE CreateTcpNodeNoRegist(uint32_t dwIP, uint16_t wPort, BOOL32 bTcpNodeReuse)
{
    SOCKHANDLE tSock = INVALID_SOCKET;
    SOCKADDR_IN tSvrINAddr;
    uint32_t optVal = 0;

    memset( &tSvrINAddr, 0, sizeof(tSvrINAddr) );

    // set up the local address
    tSvrINAddr.sin_family = AF_INET;
    tSvrINAddr.sin_port = htons(wPort);
    tSvrINAddr.sin_addr.s_addr = INADDR_ANY;

    //Allocate a socket
    tSock = socket(AF_INET, SOCK_STREAM, 0);
    if(tSock == INVALID_SOCKET)
    {
        OspLog(1, "\nOsp: Tcp server can't create socket!\n");
        return INVALID_SOCKET;
    }

    /*set the sock can reuserd immediated*/
    if(bTcpNodeReuse)
    {
        optVal = 1;
        setsockopt(tSock, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal));
    }

    if(bind(tSock, (SOCKADDR *)&tSvrINAddr, sizeof(tSvrINAddr)) == SOCKET_ERROR)
    {
        OspLog(1, "\nOsp: PassiveTcp: bind error!\n");
        OspSockClose(tSock);
        return INVALID_SOCKET;
    }

    if(listen(tSock, 15) == SOCKET_ERROR) // max 15 waiting connection
    {
        OspLog(1, "\nOsp: PassiveTcp can't listen on port = %d!\n", wPort);
        OspSockClose(tSock);
        return INVALID_SOCKET;
    }

    return tSock;
}

/*====================================================================
  函数名：SockInit
  功能：套接口库初始化。封装windows的WSAStartup，vxWorks下返回TRUE.
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：

  返回值说明：成功返回TRUE，失败返回FALSE
  ====================================================================*/
API BOOL32 OspSockInit(void)
{
#ifdef _MSC_VER
    WSADATA wsaData;
    uint32_t err;

    err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(err != 0)
    {
	return FALSE;
    }
#endif

    return TRUE;
}

/*====================================================================
  函数名：SockSend
  功能：向socket发送消息
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：

  返回值说明：
  ====================================================================*/
BOOL32 SockSend(SOCKHANDLE tSock, const char * pchBuf, uint32_t dwLen)
{   
    int ret = SOCKET_ERROR;
    uint32_t nTotalSendLen = 0;
    int nTryNum;
#ifdef _MSC_VER
    nErrNo;
#endif

    if((tSock == INVALID_SOCKET) || (pchBuf == NULL))
    {
        return FALSE;
    }

    
    nTotalSendLen = 0;
    while (nTotalSendLen < dwLen)
    {
        //发送失败原因为底层没有Buf时，要重新尝试发送3次
        for(nTryNum = 0; nTryNum < 3; nTryNum++)
        {
            ret = send(tSock, (char*)(pchBuf + nTotalSendLen), dwLen - nTotalSendLen, SOCK_SEND_FLAGS);
            if(ret == SOCKET_ERROR)
            {
#ifdef _MSC_VER
                nErrNo = OSPGetSockError();
                if(nErrNo != WSAENOBUFS)
                {
                    printf("Osp: SockSend error : %d\n", nErrNo); 
                    return FALSE;
                }

                //test by gzj.
                printf("[Osp]SockSend: nobuf now.retry(%u) times.\n",nTryNum);

                OspDelay(50);
#endif
#ifdef _VXWORKS_
                if(errno != ENOBUFS)
                {
                    printf("Osp: SockSend error : %d\n", errno); 
                    return FALSE;
                }
                OspTaskDelay(50);
#endif
            }
            else
            {
                break;
            }
        }
        nTotalSendLen += ret;
    }

    //test by gzj.
  //  OspLog(1,"[Osp]SockSend: actually: sock=%d, len=%u.\n", tSock, nTotalSendLen);
    return TRUE;
  
}

/*====================================================================
  函数名：SockRecv
  功能：从socket接收消息
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：

  返回值说明：
  ====================================================================*/
BOOL32 SockRecv(SOCKHANDLE tSock, char * pchBuf, uint32_t dwLen, uint32_t *puRcvLen)
{
    int32_t ret = SOCKET_ERROR;	

    if ((tSock == INVALID_SOCKET) || (pchBuf == NULL))
    {
        return FALSE;
    }

    // 接收数据大小若为零，则立即返回；
    // 否则，会导致recv也返回零，导致语义冲突
    if (dwLen == 0)
    {
        if(puRcvLen != NULL) *puRcvLen = 0;
        return TRUE;
    }

    ret = recv(tSock, pchBuf, dwLen, 0);
    if(SOCKET_ERROR == ret || 0 == ret)	
    {
        OspPrintf(TRUE, FALSE, "Osp: sock receive error, errno %d\n", OSPGetSockError()); 
        return FALSE;
    }

    if(puRcvLen != NULL) *puRcvLen = (uint32_t)ret;

    //test by gzj.
    OspLog(1,"[osp] SockRecv:  sock(%u).retlen(%u)\n",tSock,ret);

    return TRUE; 
 }

/*====================================================================
  函数名：SockCleanup
  功能：套接口库清理，封装windows的WSACleanup，vxWorks下返回TRUE
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：

  返回值说明：成功返回TRUE，失败返回FALSE
  ====================================================================*/
API BOOL32 OspSockCleanup(void)
{
#ifdef _MSC_VER
    uint32_t err;
    err = WSACleanup();
    if(err != 0) return FALSE;	
#endif

    return TRUE;
}

/*====================================================================
  函数名：SockShutdown
  功能：对各平台shutdown的简单封装
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：hSock--要操作的套接字，
  dwMode--操作模式, 可取值为STOP_SEND, STOP_RECV或STOP_BOTH

  返回值说明：成功返回TRUE，失败返回FALSE
  ====================================================================*/
#ifdef _LINUX_
API BOOL32 OspSockShutdown(SOCKHANDLE hSock, uint32_t dwMode)
{

    int nRet = -1;
    if(hSock == INVALID_SOCKET)
    {
        return FALSE;
    }
    switch(dwMode)
    {
    case STOP_SEND:
	nRet = shutdown(hSock, SHUT_WR);
	break;

    case STOP_RECV:
	nRet = shutdown(hSock, SHUT_RD);
	break;

    case STOP_BOTH:
	nRet = shutdown(hSock, SHUT_RDWR);
	break;

    default:
	break;
    }	
    return (nRet==0); 
}
#endif

/*====================================================================
函数名：SockClose
功能：关闭套接字，windows下封装closesocket，vxWorks下封装close
算法实现：（可选项）
引用全局变量：
输入参数说明：

  返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
API BOOL32 OspSockClose(SOCKHANDLE hSock)
{
    if(hSock == INVALID_SOCKET)
    {
        return FALSE;
    }
    
#ifdef _MSC_VER
    return ( 0 == closesocket(hSock) ); 
#endif
    
#ifdef _VXWORKS_
    /* 不准用户关闭标准输入\输出 */
    if(hSock < 3)
    {
        return FALSE;
    }
    
    return ( OK == close(hSock) );
#endif
    
#ifdef _LINUX_
    /* 不准用户关闭标准输入\输出 */
    if(hSock < 3)
    {
        return FALSE;
    }
    
    return ( 0 == close(hSock) );	
#endif
}

int32_t OSPGetSockError(void)
{
#ifdef _MSC_VER
    return WSAGetLastError ();
#else
    return errno;
#endif
}

void OSPSetSockNoBlock(SOCKHANDLE sock)
{
	uint32_t on = TRUE;
	
#ifdef WIN32
	ioctlsocket(sock, FIONBIO, &on);
#else
	ioctl(sock, FIONBIO, &on);
#endif

	return;
}

