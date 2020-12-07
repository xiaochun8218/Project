/*****************************************************************************
*  DDBS Network Byte Order												     *
*  Copyright (C) 2019                                                        *
*                                                                            *
*  @file     net_byte_order.h                                                    *
*  @brief    ÍøÂç×Ö½ÚÐò                                                  *
*  Details.                                                                  *
*                                                                            *
*  @author   ÐìÐ¡À× <XuXiaoLei>                                              *
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
#ifndef __DDBS_NET_BYTE_ORDER_H__
#define __DDBS_NET_BYTE_ORDER_H__

namespace datacenter
{
#define BigLittleSwap16(A) ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8))
#define BigLittleSwap32(A) ((((uint32_t)(A) & 0xff000000) >> 24) | (((uint32_t)(A) & 0x00ff0000) >> 8) | (((uint32_t)(A) & 0x0000ff00) << 8) | (((uint32_t)(A) & 0x000000ff) << 24))
#define BigLittleSwap64(A) ((((uint64_t)BigLittleSwap32((uint32_t)(((A) << 32) >> 32))) << 32) | (uint32_t)BigLittleSwap32((uint32_t)((A) >> 32)))
#define BigLittleSwap64double(S,D) (*((uint64_t*)(D)) = BigLittleSwap64(*((uint64_t*)(&(S)))))
}

#endif