#include "CEvent.h"

namespace datacenter
{
    CEvent::CEvent()
        :m_flag(false)
    {
#ifdef WIN32
        m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_lock = CreateMutex(NULL, FALSE, NULL);
#else
        pthread_cond_init(&m_event, NULL);
        pthread_mutex_init(&m_lock, NULL);
#endif
    }

    CEvent::~CEvent()
    {
#ifdef WIN32
        CloseHandle(m_event);
        CloseHandle(m_lock);
#else
        pthread_cond_destroy(&m_event);
        pthread_mutex_destroy(&m_lock);
#endif
    }

    void CEvent::Lock()
    {
#ifdef  WIN32
        WaitForSingleObject(m_lock, INFINITE);
#else
        pthread_mutex_lock(&m_lock);
#endif
    }

    void CEvent::UnLock()
    {
#ifdef  WIN32
        ReleaseMutex(m_lock);
#else
        pthread_mutex_unlock(&m_lock);
#endif
    }

    void CEvent::Wait()
    {
#ifdef WIN32
        Lock();
        WaitForSingleObject(m_event, INFINITE);
        UnLock();
#else
        pthread_mutex_lock(&m_lock);
        while (!m_flag)
        {
            pthread_cond_wait(&m_event, &m_lock);
        }
        m_flag = false;
        pthread_mutex_unlock(&m_lock);
#endif
    }

    void CEvent::Release()
    {
#ifdef WIN32
        ::SetEvent(m_event);
#else
        pthread_mutex_lock(&m_lock);
        m_flag = true;
        pthread_mutex_unlock(&m_lock);
        pthread_cond_signal(&m_event);
#endif
    }

    bool CEvent::WaitForTime(int wait_ms /*ºÁÃë*/)
    {
#ifdef WIN32
        Lock();
        if (WaitForSingleObject(m_event, wait_ms) == WAIT_OBJECT_0)
        {
            UnLock();
            return true;
        }
        else
        {
            UnLock();
            return false;
        }
#else
        struct timespec tms;
        struct timeval  now;
        int usecadd = (wait_ms % 1000) * 1000;

        gettimeofday(&now, NULL);
        now.tv_usec += usecadd;
        if (now.tv_usec >= 1000000)
        {
            now.tv_sec += (now.tv_usec / 1000000);
            now.tv_usec %= 1000000;
        }

        tms.tv_sec = wait_ms / 1000 + now.tv_sec;
        tms.tv_nsec = now.tv_usec * 1000;
        pthread_mutex_lock(&m_lock);
        if (pthread_cond_timedwait(&m_event, &m_lock, &tms) == 0)
        {
            if (m_flag)
            {
                m_flag = false;
                pthread_mutex_unlock(&m_lock);
                return true;
            }
        }
        pthread_mutex_unlock(&m_lock);
        return false;
#endif
    }

    CLock::CLock()
    {
#ifdef WIN32
        InitializeCriticalSection(&m_section);
#else
        pthread_mutexattr_t mattr;
        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_section, &mattr);
        pthread_mutexattr_destroy(&mattr);
#endif
    }

    CLock::~CLock()
    {
#ifdef WIN32
        DeleteCriticalSection(&m_section);
#else
        pthread_mutex_destroy(&m_section);
#endif
    }
}
