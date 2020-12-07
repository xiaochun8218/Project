/*****************************************************************************
*  CLog for DDB                                                              *
*  Copyright (C) 2019                                                        *
*                                                                            *
*  @file     log.h														     *
*  @brief    日志类                                                          *
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
#ifndef __XXL_LOG_H__
#define __XXL_LOG_H__

#include "inc.h"
#include "CEvent.h"

// 最长的LOG文件长度限制
#define MAXLOGLENGTH 1024 * 1024 * 256

namespace datacenter
{
    // LOG级别
    enum class LOG_LEVEL :uint8_t
    {
        LOG_ERROR = 0,
        LOG_WARN,
        LOG_INFO,
        LOG_DEBUG
    };

    const char* const log_level[] =
    {
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG"
    };

    //是否打印控制台日志
    enum class CONSOLE_PRINT
    {
        NO = 0,
        YES
    };

    class CLog
    {
    public:
        CLog();
        ~CLog();

        bool Open(char* LogFileName = NULL);
        void Close();
        void Log(LOG_LEVEL level, CONSOLE_PRINT printConsole, const char* pFormat, ...);
        void SetLogPath(const char* path);
        void SetLevel(LOG_LEVEL level);
        void SetOverdue(int overdue);
        void SetPrintConsoleFlag(CONSOLE_PRINT flag);

    private:
        bool checkfile(int date);
        bool openfile();
        void closefile();
        int getfileNumber(const char* prename);
        void rmOverduefile();

    public:
        static int gettime(char* t = NULL);

    private:
        int m_FileNumber;
        char m_Prefix[100];
        char m_LogPath[PATH_MAX - 256];
        int m_LogDate;
        long long m_CurFileLen;
        LOG_LEVEL m_Level;
        int m_Overdue;
        CONSOLE_PRINT m_PrintConsoleFlag;

        FILE* m_fp;
        CLock m_Lock;
    };
}

#define LOG_MSG_WARN(ptrLog, fmt, ...)                                 \
    {                                                                  \
        if (ptrLog)                                                    \
        {                                                              \
            (ptrLog)->Log(datacenter::LOG_LEVEL::LOG_WARN, datacenter::CONSOLE_PRINT::NO, fmt, ##__VA_ARGS__); \
        }                                                              \
    }

#define LOG_MSG_INFO(ptrLog, fmt, ...)                                 \
    {                                                                  \
        if (ptrLog)                                                    \
        {                                                              \
            (ptrLog)->Log(datacenter::LOG_LEVEL::LOG_INFO, datacenter::CONSOLE_PRINT::NO, fmt, ##__VA_ARGS__); \
        }                                                              \
    }

#define LOG_MSG_DEBUG(ptrLog, fmt, ...)                                 \
    {                                                                   \
        if (ptrLog)                                                     \
        {                                                               \
            (ptrLog)->Log(datacenter::LOG_LEVEL::LOG_DEBUG, datacenter::CONSOLE_PRINT::NO, fmt, ##__VA_ARGS__); \
        }                                                               \
    }

#define LOG_MSG_ERROR(ptrLog, fmt, ...)                                 \
    {                                                                   \
        if (ptrLog)                                                     \
        {                                                               \
            (ptrLog)->Log(datacenter::LOG_LEVEL::LOG_ERROR, datacenter::CONSOLE_PRINT::NO, fmt, ##__VA_ARGS__); \
        }                                                               \
    }

#endif
