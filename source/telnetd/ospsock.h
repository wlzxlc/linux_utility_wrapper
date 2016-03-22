#ifndef _OSPSOCK_H_
#define _OSPSOCK_H_
#include <ccstone/telnetd/ospcommon.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif 

#define SOCKHANDLE int

#define	INVALID_SOCKET  -1
#define SOCKET_ERROR -1
#ifdef _LINUX_
#define SOCK_SEND_FLAGS             (int)MSG_NOSIGNAL
typedef socklen_t                   socklen;
#define MAX_SEM_PER_PROCESS 1024

#else
	#define SOCK_SEND_FLAGS             (int)0
	typedef int                         socklen;
#endif

    // 256K; SOCKET的发送缓冲大小
#define SOCKET_SEND_BUF					1024 * 256
    // 256K; SOCKET的接收缓冲大小
#define SOCKET_RECV_BUF					1024 * 256

#define SOCKADDR_IN 		 struct sockaddr_in
typedef struct sockaddr * SOCKADDRPTR;
typedef struct sockaddr  SOCKADDR;
BOOL32 SockSend(SOCKHANDLE sock,const char * buf, uint32_t len);
BOOL32 OspSockClose(SOCKHANDLE hSock);
BOOL32 SockRecv(SOCKHANDLE sock, char * buf, uint32_t len, uint32_t * pRcvLen);

int32_t OSPGetSockError(void);

void OSPSetSockNoBlock(SOCKHANDLE sock);
SOCKHANDLE CreateTcpNodeNoRegist(uint32_t dwIP, uint16_t wPort, BOOL32 bTcpNodeReuse);




#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
