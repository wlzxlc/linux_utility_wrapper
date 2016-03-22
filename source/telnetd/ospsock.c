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
  ��������SockInit
  ���ܣ��׽ӿڿ��ʼ������װwindows��WSAStartup��vxWorks�·���TRUE.
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����

  ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
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
  ��������SockSend
  ���ܣ���socket������Ϣ
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����

  ����ֵ˵����
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
        //����ʧ��ԭ��Ϊ�ײ�û��Bufʱ��Ҫ���³��Է���3��
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
  ��������SockRecv
  ���ܣ���socket������Ϣ
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����

  ����ֵ˵����
  ====================================================================*/
BOOL32 SockRecv(SOCKHANDLE tSock, char * pchBuf, uint32_t dwLen, uint32_t *puRcvLen)
{
    int32_t ret = SOCKET_ERROR;	

    if ((tSock == INVALID_SOCKET) || (pchBuf == NULL))
    {
        return FALSE;
    }

    // �������ݴ�С��Ϊ�㣬���������أ�
    // ���򣬻ᵼ��recvҲ�����㣬���������ͻ
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
  ��������SockCleanup
  ���ܣ��׽ӿڿ�������װwindows��WSACleanup��vxWorks�·���TRUE
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����

  ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
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
  ��������SockShutdown
  ���ܣ��Ը�ƽ̨shutdown�ļ򵥷�װ
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����hSock--Ҫ�������׽��֣�
  dwMode--����ģʽ, ��ȡֵΪSTOP_SEND, STOP_RECV��STOP_BOTH

  ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
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
��������SockClose
���ܣ��ر��׽��֣�windows�·�װclosesocket��vxWorks�·�װclose
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

  ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
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
    /* ��׼�û��رձ�׼����\��� */
    if(hSock < 3)
    {
        return FALSE;
    }
    
    return ( OK == close(hSock) );
#endif
    
#ifdef _LINUX_
    /* ��׼�û��رձ�׼����\��� */
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

