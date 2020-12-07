/*****************************************************************************
*  MUTEX for DDB                                                             *
*  Copyright (C) 2019                                                        *
*                                                                            *
*  @file     mutex.h														 *
*  @brief    线程同步相关的事件、锁                                          *
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
#ifndef __MY_MUTEX_H__
#define __MY_MUTEX_H__

#include "inc.h"

namespace datacenter
{
    class CEvent
    {
    public:
        CEvent();
        virtual ~CEvent();

        void Lock();
        void UnLock();
        void Wait();
        bool WaitForTime(int wait_ms/*毫秒*/);
        void Release();

    private:
        bool m_flag;
#ifdef WIN32
        HANDLE m_event;
        HANDLE m_lock;
#else
        pthread_cond_t m_event;
        pthread_mutex_t m_lock;
#endif
    };

    class CLock
    {
    public:
        CLock();
        ~CLock();

        inline void Lock()
        {
#ifdef WIN32
            EnterCriticalSection(&m_section);
#else
            pthread_mutex_lock(&m_section);
#endif
        }

        inline void UnLock()
        {
#ifdef WIN32
            LeaveCriticalSection(&m_section);
#else
            pthread_mutex_unlock(&m_section);
#endif
        }
    public:
#ifdef WIN32
        CRITICAL_SECTION m_section;
#else
        pthread_mutex_t m_section;
#endif
    };

    class CSRWLock
    {
    public:
        CSRWLock()
        {
#ifdef WIN32
            InitializeSRWLock(&m_srwlock);
#else
            pthread_rwlock_init(&m_rwlock, NULL);
#endif
        }

        ~CSRWLock()
        {
#ifdef WIN32
#else
            pthread_rwlock_destroy(&m_rwlock);
#endif
        }

        inline void SRLock()
        {
#ifdef WIN32
            AcquireSRWLockShared(&m_srwlock);
#else
            pthread_rwlock_rdlock(&m_rwlock);
#endif
        }

        inline void WLock()
        {
#ifdef WIN32
            AcquireSRWLockExclusive(&m_srwlock);
#else
            pthread_rwlock_wrlock(&m_rwlock);
#endif
        }

        inline void UnSRLock()
        {
#ifdef WIN32
            ReleaseSRWLockShared(&m_srwlock);
#else
            pthread_rwlock_unlock(&m_rwlock);
#endif
        }

        inline void UnWLock()
        {
#ifdef WIN32
            ReleaseSRWLockExclusive(&m_srwlock);
#else
            pthread_rwlock_unlock(&m_rwlock);
#endif
        }
    private:
#ifdef WIN32
        SRWLOCK m_srwlock;
#else
        pthread_rwlock_t m_rwlock;
#endif
    };

    template<class T>
    class CLockGuard
    {
    public:
        CLockGuard(T& mtx) : m_mtx(mtx) {
            m_mtx.Lock();
        };

        ~CLockGuard() {
            m_mtx.UnLock();
        };
    private:
        T& m_mtx;
    };
}

#endif
