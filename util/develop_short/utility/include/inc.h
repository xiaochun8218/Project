/*****************************************************************************
*  inc for DDB                                                               *
*  Copyright (C) 2019                                                        *
*                                                                            *
*  @file     inc.h                                                           *
*  @brief    系统需引入的文件                                                *
*  Details.                                                                  *
*                                                                            *
*  @author   徐小雷 <XuXiaoLei>                                              *
*  @date     2019-11-25                                                      *
*  @license                                                                  *
*                                                                            *
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2019-11-25 | 1.0.0.0   | XuXiaoLie      | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/
#ifndef __INC_H__
#define __INC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <set>
#include <time.h>
#include <vector>
#include <algorithm>

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
#include <direct.h>
#include "dirent_win.h"
#define socket_errno h_errno
#define socklen_t int
typedef SOCKET SOCKET_HANDLE;
#define EWOULDBLOCK WSAEWOULDBLOCK
#define PATHMARK  '\\'
#define MAKEPATH(a) _mkdir(a)
#include <share.h>
#define sh_fopen(a,b,c) _fsopen(a,b,c)
#include <io.h>
#define sh_open(a,b,c)  _sopen(a,b|O_BINARY,c,S_IREAD | S_IWRITE)
#define snprintf _snprintf
#define pthread_t DWORD
#else
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <bits/socket.h>
#include <fcntl.h>
#include <asm-generic/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <limits.h>
#include <dirent.h>
#define socket_errno errno

#define PATHMARK  '/'
#define MAKEPATH(a) mkdir(a,0777)
#define SH_DENYNO    0x40
#define SH_DENYWR    0x20
#define sh_fopen(a,b,c) fopen(a,b)
#define sh_open(a,b,c)  open(a,b,S_IREAD | S_IWRITE)
#define closesocket close
#define ioctlsocket ioctl
typedef int SOCKET_HANDLE;

#endif

#ifndef INVALID_SOCKET
#define	INVALID_SOCKET	(-1)
#endif

#endif
