/*****************************************************************************
*  DDBS Function															 *
*  Copyright (C) 2019                                                        *
*                                                                            *
*  @file     function.hpp													 *
*  @brief    ������													         *
*  Details.                                                                  *
*                                                                            *
*  @author   ��С�� <XuXiaoLei>                                              *
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
#ifndef __DDBS_FUNCTION_H__
#define __DDBS_FUNCTION_H__

#include <string>
#include "inc.h"
#include "CEvent.h"


#define SAFEDELETE(p) \
if(p) \
{\
    delete p;\
    p = NULL;\
}

#define SAFEFREE(p) \
if(p) \
{ \
    free(p); \
    p = NULL;\
}

#define SAFEDELETEARR(p) \
if(p) \
{ \
    delete [] p; \
    p = NULL;\
}

#define WRITE_TIMESTAMP(A) {\
	struct timespec ts = { 0, 0 }; \
	clock_gettime(CLOCK_REALTIME, &ts); \
	*((long long*)(A)) = ts.tv_sec * 1000000000 + ts.tv_nsec; \
}

#ifdef WIN32
#include <windows.h>
#define msleep(msec) Sleep(msec)
#else
#include <unistd.h>
#define msleep(msec) usleep(msec * 1000)
#endif // WIN32

namespace datacenter
{
#ifdef WIN32
    pthread_t StartThread(LPTHREAD_START_ROUTINE start_rtn, void* pParam, int priority = 60);
#else
    pthread_t StartThread(void* (*start_rtn)(void*), void* pParam, int priority = 60);
#endif

#ifdef WIN32
    int gettimeofday(struct timeval* tp, void* tzp);
#endif

    /*
    * ʱ�������ȷ��us
    */
    int64_t gettimestamp();
    int get_date();
    int get_time();

    /**
     * @return HHmmssSSS��ʽ��ʱ��.
     */
    int64_t get_time_ms();
    void get_date_time(char* d, char* t);
    int time_to_int(char* t);

    /*!
     *
     * @brief           ��yyyyMMddHHmmssSSS��ʽ��תΪʱ���.
     * @param[in] time  yyyyMMddHHmmssSSS��ʽ��ʱ��
     * @return          ��ȷ��΢����ʱ���
     * @retval int64_t
     *
     */
    int64_t convert_to_timestamp(int64_t time);

    /*!
     *
     * @brief           ��13λʱ���תΪyyyyMMddHHmmssSSS��ʽ��ʱ��.
     * @param[in] time  ��ȷ��΢����ʱ���
     * @return          yyyyMMddHHmmssSSS��ʽ��ʱ��
     * @retval int64_t
     *
     */
    int64_t convert_to_datetime(int64_t timestamp);

    /*!
     *
     * @brief       ��ȡ��ǰʱ��.
     * @return      yyyyMMddHHmmssSSS��ʽʱ��
     * @retval int64_t
     *
     */
    int64_t get_datetime_ms();

    void trim_memcpy(char* dest, int dest_size, const char* src, int src_size);
    void trim(char* str);
    void trim_left(char* str, char c);

    /*!
     *
     * @brief                   �ֶ��з�.
     * @param[in] str           ԭʼ����
     * @param[in] split_flag    �ָ��
     * @return                  ���ָ����ָ�֮�������
     * @retval std::vector<std::string>
     *
     */
    std::vector<std::string> str_split(const char* str, const char* split_flag);
    int to_2power(int n);
    bool SetThreadAffinity(int id, const char* name, int& nproces, int& set_num);
    bool SetThreadAffinity(int id, const char* name);
    int GetSystemCPUCoreNum();
    int double_compare(double a, double b);


    /**
     * \brief           ����������������
     * \param value     ����
     * \param digits    ����С��λ��
     * \return          ��������������
     */
    double round(double value, int digits);

    class CThreadMgr
    {
    public:
        CThreadMgr() :m_Count(0), m_ExitFlag(false)
        {
        }

        ~CThreadMgr()
        {
        }

        inline bool IsExit()
        {
            return m_ExitFlag;
        }

        inline void Exit()
        {
            m_ExitFlag = true;
            m_ExitEvent.Wait();
        }

        inline int Join()
        {
#ifdef WIN32
            return InterlockedIncrement((unsigned int*)&m_Count);
#else
            return __sync_add_and_fetch(&(m_Count), 1);
#endif
        }

        inline void UnJoin()
        {
#ifdef WIN32
            if (0 == InterlockedDecrement((unsigned int*)&m_Count))
#else
            if (0 == __sync_sub_and_fetch(&(m_Count), 1))
#endif
            {
                m_ExitEvent.Release();
            }
        }
    private:
        CEvent m_ExitEvent;
        int m_Count;
        bool m_ExitFlag;
    };

    /*!
     *
     * @brief               ��ȡ��������Ŀ����.
     * @return              ��Ŀ����
     * @retval const char*
     *
     */
    const char* getProjectName();

    /*!
     *
     * @brief               ��ȡ�����İ汾��.
     * @return              �汾��
     * @retval const char*
     *
     */
    const char* getVersion();

    /*!
     *
     * @brief           ��ȡ��������ʱ��.
     * @return          ����ʱ��
     * @retval const char*
     *
     */
    const char* getBuildTime();

    /*!
     *
     * @brief           ��ȡ������Ӧgit�汾.
     * @return          ��Ӧgit�汾
     * @retval const char*
     *
     */
    const char* getGitRevision();
}

#endif
