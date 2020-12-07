/*****************************************************************************
*  DDBS Function															 *
*  Copyright (C) 2019                                                        *
*                                                                            *
*  @file     function.hpp													 *
*  @brief    函数库													         *
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
    * 时间戳，精确到us
    */
    int64_t gettimestamp();
    int get_date();
    int get_time();

    /**
     * @return HHmmssSSS格式的时间.
     */
    int64_t get_time_ms();
    void get_date_time(char* d, char* t);
    int time_to_int(char* t);

    /*!
     *
     * @brief           将yyyyMMddHHmmssSSS格式的转为时间戳.
     * @param[in] time  yyyyMMddHHmmssSSS格式的时间
     * @return          精确到微妙级别的时间戳
     * @retval int64_t
     *
     */
    int64_t convert_to_timestamp(int64_t time);

    /*!
     *
     * @brief           将13位时间戳转为yyyyMMddHHmmssSSS格式的时间.
     * @param[in] time  精确到微妙级别的时间戳
     * @return          yyyyMMddHHmmssSSS格式的时间
     * @retval int64_t
     *
     */
    int64_t convert_to_datetime(int64_t timestamp);

    /*!
     *
     * @brief       获取当前时间.
     * @return      yyyyMMddHHmmssSSS格式时间
     * @retval int64_t
     *
     */
    int64_t get_datetime_ms();

    void trim_memcpy(char* dest, int dest_size, const char* src, int src_size);
    void trim(char* str);
    void trim_left(char* str, char c);

    /*!
     *
     * @brief                   字段切分.
     * @param[in] str           原始数据
     * @param[in] split_flag    分割符
     * @return                  按分隔符分割之后的数据
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
     * \brief           正数数据四舍五入
     * \param value     正数
     * \param digits    保留小数位数
     * \return          四舍五入后的数据
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
     * @brief               获取该类库的项目名称.
     * @return              项目名称
     * @retval const char*
     *
     */
    const char* getProjectName();

    /*!
     *
     * @brief               获取该类库的版本号.
     * @return              版本号
     * @retval const char*
     *
     */
    const char* getVersion();

    /*!
     *
     * @brief           获取该类库编译时间.
     * @return          编译时间
     * @retval const char*
     *
     */
    const char* getBuildTime();

    /*!
     *
     * @brief           获取该类库对应git版本.
     * @return          对应git版本
     * @retval const char*
     *
     */
    const char* getGitRevision();
}

#endif
