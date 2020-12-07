#include <sys/timeb.h>
#include "log.h"


namespace datacenter
{
    ///////////////////////////////////////////////////////////////////////////////////
    CLog::CLog()
    {
        m_FileNumber = 0;
        m_fp = NULL;
        m_LogDate = 0;
        m_CurFileLen = 0;
        m_Level = LOG_LEVEL::LOG_ERROR;
        m_PrintConsoleFlag = CONSOLE_PRINT::NO;
        m_Overdue = 10;
        memset(m_Prefix, 0, sizeof(m_Prefix));
        memset(m_LogPath, 0, sizeof(m_LogPath));
    }

    CLog::~CLog()
    {
        Close();
    }

    void CLog::SetLevel(LOG_LEVEL level)
    {
        m_Level = level;
    }

    void CLog::SetLogPath(const char* path)
    {
        if (NULL == path || nullptr == path)
        {
            strncpy(m_LogPath, "./logs", sizeof(m_LogPath) - 1);
        }
        else
        {
            strncpy(m_LogPath, path, sizeof(m_LogPath) - 1);
        }
    }

    void CLog::SetPrintConsoleFlag(CONSOLE_PRINT flag)
    {
        m_PrintConsoleFlag = flag;
    }

    void CLog::SetOverdue(int overdue)
    {
        m_Overdue = overdue;
    }

    int CLog::gettime(char* t)
    {
#ifdef WIN32
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        if (t != NULL)
        {
            sprintf(t, "%02d:%02d:%02d.%03d", wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds);
        }

        return wtm.wYear * 10000 + wtm.wMonth * 100 + wtm.wDay;
#else
        struct tm tmloc;
        struct timeb raw_time;
        ftime(&raw_time);
        localtime_r(&raw_time.time, &tmloc);
        if (t != NULL)
        {
            sprintf(t, "%02d:%02d:%02d.%03d", tmloc.tm_hour, tmloc.tm_min, tmloc.tm_sec, raw_time.millitm);
        }

        return (tmloc.tm_year + 1900) * 10000 + (tmloc.tm_mon + 1) * 100 + tmloc.tm_mday;
#endif // WIN32
    }

    bool CLog::Open(char* logName_Prefix)
    {
        CLockGuard<CLock> guard(m_Lock);

        if (m_fp != NULL)
        {
            return false;
        }

        if (logName_Prefix != NULL)
        {
            memset(m_Prefix, 0, sizeof(m_Prefix));
            strncpy(m_Prefix, logName_Prefix, sizeof(m_Prefix) - 1);
        }

        return openfile();
    }

    bool CLog::openfile()
    {
        char fname[PATH_MAX];
        char prename[128];
        m_LogDate = gettime();
        m_CurFileLen = 0;
        memset(prename, 0, sizeof(prename));
        if (strlen(m_Prefix) > 0)
        {
            snprintf(prename, sizeof(prename) - 1, "%s_%d_", m_Prefix, m_LogDate);
        }
        else
        {
            snprintf(prename, sizeof(prename) - 1, "%d_", m_LogDate);
        }

        m_FileNumber = getfileNumber(prename);
        if (-1 == m_FileNumber)
        {
            return false;
        }

        while (true)
        {
            memset(fname, 0, sizeof(fname));
            snprintf(fname, sizeof(fname) - 1, "%s%c%s%d.log", m_LogPath, PATHMARK, prename, m_FileNumber++);
#ifdef WIN32

            if (_access(fname, 0) != 0)
#else
            if (access(fname, 0) != 0)
#endif

            {
                break;
            }
        }

        m_fp = sh_fopen(fname, "a+t", SH_DENYNO);
        if (m_fp == NULL)
        {
            m_fp = sh_fopen(fname, "w+t", SH_DENYNO);
            if (m_fp == NULL)
            {
                return false;
            }
        }
        rmOverduefile();
        return true;
    }

    void CLog::Close()
    {
        CLockGuard<CLock> guard(m_Lock);
        closefile();
    }

    void CLog::closefile()
    {
        if (m_fp != NULL)
        {
            fclose(m_fp);
            m_fp = NULL;
            m_CurFileLen = 0;
            m_LogDate = 0;
        }
    }

    void CLog::Log(LOG_LEVEL level, CONSOLE_PRINT printConsole, const char* pFormat, ...)
    {
        if (level > m_Level)
        {
            return;
        }

        char str_t[32];
        memset(str_t, 0, sizeof(str_t));
        int date = gettime(str_t);
        int d_len = strlen(str_t);
        int year = date / 10000;
        int month = (date - year * 10000) / 100;
        int day = date - (date / 100) * 100;

        va_list pArg;

        va_start(pArg, pFormat);
        int formatlen = vsnprintf(NULL, 0, pFormat, pArg) + d_len + 32;
        char* buf = new char[formatlen];
        if (buf == NULL)
        {
            return;
        }
        memset(buf, 0, formatlen);
        va_start(pArg, pFormat);
        sprintf(buf, "%d-%02d-%02d %s [%s]", year, month, day, str_t, log_level[(int)level]);
        int pre_len = strlen(buf);
        buf[pre_len] = ' ';
        ++pre_len;
        vsnprintf(buf + pre_len, formatlen - pre_len - 1, pFormat, pArg);
        buf[strlen(buf)] = '\n';

        va_end(pArg);

        {
            CLockGuard<CLock> guard(m_Lock);
            if (!checkfile(date))
            {
                closefile();
                if (!openfile())
                {
                    return;
                }
            }
            m_CurFileLen += strlen(buf);
            fputs(buf, m_fp);
            fflush(m_fp);
        }

        if (CONSOLE_PRINT::YES == m_PrintConsoleFlag || CONSOLE_PRINT::YES == printConsole)
        {
            printf("%s", buf);
        }
        delete buf;
    }

    bool CLog::checkfile(int date)
    {
        if (m_fp == NULL || m_LogDate != date || m_CurFileLen >= MAXLOGLENGTH)
        {
            return false;
        }
        return true;
    }

    int CLog::getfileNumber(const char* prename)
    {
#ifdef WIN32
        if (_access(m_LogPath, 0) != 0)
#else
        if (access(m_LogPath, 0) != 0)
#endif

        {
            MAKEPATH(m_LogPath);
            return 0;
        }

        DIR* dir;
        if ((dir = opendir(m_LogPath)) == NULL)
        {
            return -1;
        }

        int number = 0;
        struct dirent* pDirent;
        size_t pernamelen = strlen(prename);
        while ((pDirent = readdir(dir)) != NULL)
        {
            if (pDirent->d_type == 8)
            {
                if (strncmp(prename, pDirent->d_name, pernamelen) == 0)
                {
                    strrchr(pDirent->d_name, '.')[0] = 0;
                    int n = atoi(strrchr(pDirent->d_name, '_') + 1);
                    if (n > number)
                    {
                        number = n;
                    }
                }
            }
        }
        return number;
    }

    void CLog::rmOverduefile()
    {
        if (0 == m_LogDate)
        {
            return;
        }

        int year = m_LogDate / 10000;
        int month = (m_LogDate - (year * 10000)) / 100;
        int day = m_LogDate - (year * 10000) - (month * 100);

        tm info = { 0 };
        info.tm_year = year - 1900;
        info.tm_mon = month - 1;
        info.tm_mday = day;
        time_t tt = mktime(&info);

        tt -= (24 * 3600 * m_Overdue);
        tm* ptm = localtime(&tt);
        int destdate = (ptm->tm_year + 1900) * 10000 + (ptm->tm_mon + 1) * 100 + ptm->tm_mday;

        DIR* dir;
        if ((dir = opendir(m_LogPath)) == NULL)
        {
            return;
        }
        struct dirent* pDirent;
        char d[9];
        memset(d, 0, sizeof(d));
        char cmd[PATH_MAX + 10];
        memset(cmd, 0, sizeof(cmd));
        while ((pDirent = readdir(dir)) != NULL)
        {
            if (pDirent->d_type == 8 && strlen(pDirent->d_name) >= 14)
            {
                if (strlen(m_Prefix) == 0)
                {
                    strncpy(d, pDirent->d_name, 8);
                }
                else
                {
                    if (strlen(pDirent->d_name) > strlen(m_Prefix) && strncmp(m_Prefix, pDirent->d_name, strlen(m_Prefix)) != 0)
                    {
                        continue;
                    }

                    strncpy(d, pDirent->d_name + strlen(m_Prefix) + 1, 8);
                }

                if (atoi(d) <= destdate)
                {
                    snprintf(cmd, sizeof(cmd) - 1, "rm -rf %s/%s", m_LogPath, pDirent->d_name);
                    cmd[strlen(cmd)] = 0;
                    system(cmd);
                }
            }
        }

        return;
    }
}
