#include "function.h"
#include <sys/timeb.h>

namespace datacenter
{
#ifdef WIN32
    pthread_t StartThread(LPTHREAD_START_ROUTINE start_rtn, void* pParam, int priority)
#else
    pthread_t StartThread(void* (*start_rtn)(void*), void* pParam, int priority)
#endif
    {
#ifdef WIN32
        DWORD ThreadId;
        HANDLE h = ::CreateThread(NULL, 0, start_rtn, pParam, 0, &ThreadId);
        //::SetThreadPriority(h, priority);
        return ThreadId;
#else
        pthread_t hThread = 0;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        sched_param param;
        pthread_attr_getschedparam(&attr, &param);
        param.sched_priority = priority == INT_MIN || priority == INT_MAX
            ? sched_get_priority_max(SCHED_RR)
            : priority;
        pthread_attr_setschedparam(&attr, &param);
        pthread_create(&hThread, &attr, start_rtn, pParam);
        pthread_attr_destroy(&attr);
        return hThread;
#endif
    }

    bool SetThreadAffinity(int id, const char* name, int& nproces, int& set_num)
    {
#ifdef WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        nproces = info.dwNumberOfProcessors;
        set_num = id % nproces;
        if (0 == SetThreadAffinityMask(GetCurrentThread(), 1 << set_num))
        {
            return false;
        }
#else
        nproces = get_nprocs();
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        set_num = id % nproces;
        CPU_SET(set_num, &cpuset);
        if (0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
        {
            return false;
        }
        pthread_setname_np(pthread_self(), name);
#endif
        return true;
    }

    int GetSystemCPUCoreNum()
    {
#ifdef WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return info.dwNumberOfProcessors;
#else
        return get_nprocs();
#endif
    }

    bool SetThreadAffinity(int id, const char* name)
    {
#ifdef WIN32
        if (0 == SetThreadAffinityMask(GetCurrentThread(), 1 << id))
        {
            return false;
        }
#else
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(id, &cpuset);
        if (0 != pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset))
        {
            return false;
        }
        pthread_setname_np(pthread_self(), name);
#endif
        return true;
    }

#ifdef WIN32
    int gettimeofday(struct timeval* tp, void* tzp)
    {
        time_t clock;
        struct tm tm;
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        tm.tm_year = wtm.wYear - 1900;
        tm.tm_mon = wtm.wMonth - 1;
        tm.tm_mday = wtm.wDay;
        tm.tm_hour = wtm.wHour;
        tm.tm_min = wtm.wMinute;
        tm.tm_sec = wtm.wSecond;
        tm.tm_isdst = -1;
        clock = mktime(&tm);
        tp->tv_sec = clock;
        tp->tv_usec = wtm.wMilliseconds * 1000;
        return (0);
    }
#endif

    int64_t gettimestamp()
    {
        struct timeval time;

        /* 获取时间，理论到us */
        gettimeofday(&time, NULL);
        return time.tv_sec * 1000000L + time.tv_usec;
    }

    int get_date()
    {
        time_t lastftime = time(NULL);
        struct tm tmloc;
#ifdef WIN32
        localtime_s(&tmloc, &lastftime);
#else
        localtime_r(&lastftime, &tmloc);
#endif
        return ((tmloc.tm_year + 1900) * 100 + tmloc.tm_mon + 1) * 100 + tmloc.tm_mday;
    }

    int get_time()
    {
        time_t lastftime = time(NULL);
        struct tm tmloc;
#ifdef WIN32
        localtime_s(&tmloc, &lastftime);
#else
        localtime_r(&lastftime, &tmloc);
#endif
        return tmloc.tm_hour * 10000 + tmloc.tm_min * 100 + tmloc.tm_sec;
    }

    int64_t get_time_ms()
    {
#ifdef WIN32
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        return wtm.wHour * 10000000L + wtm.wMinute * 100000L + wtm.wSecond * 1000 + wtm.wMilliseconds;
#else
        struct timeb raw_time;
        ftime(&raw_time);
        struct tm tm_loc;
        localtime_r(&raw_time.time, &tm_loc);
        return tm_loc.tm_hour * 10000000L + tm_loc.tm_min * 100000L + tm_loc.tm_sec * 1000 + raw_time.millitm;
#endif
    }

    void get_date_time(char* d, char* t)
    {
        time_t lastftime = time(NULL);
        struct tm tmloc;
#ifdef WIN32
        localtime_s(&tmloc, &lastftime);
#else
        localtime_r(&lastftime, &tmloc);
#endif
        if (t != NULL)
        {
            sprintf(t, "%02d:%02d:%02d", tmloc.tm_hour, tmloc.tm_min, tmloc.tm_sec);
        }
        if (d != NULL)
        {
            sprintf(d, "%d", ((tmloc.tm_year + 1900) * 100 + tmloc.tm_mon + 1) * 100 + tmloc.tm_mday);
        }
    }

    int64_t convert_to_timestamp(int64_t time)
    {
        struct tm tm_loc = { 0 };
        int year = time / 10000000000000L;
        int mon = (time - year * 10000000000000L) / 100000000000L;
        int day = (time - year * 10000000000000L - mon * 100000000000L) / 1000000000L;
        int hour = (time - year * 10000000000000L - mon * 100000000000L - day * 1000000000L) / 10000000L;
        int min = (time - year * 10000000000000L - mon * 100000000000L - day * 1000000000L - hour * 10000000L) / 100000L;
        int sec = (time - year * 10000000000000L - mon * 100000000000L - day * 1000000000L - hour * 10000000L - min * 100000L) / 1000L;
        int usec = time % 1000;
        tm_loc.tm_year = year - 1900;
        tm_loc.tm_mon = mon - 1;
        tm_loc.tm_mday = day;
        tm_loc.tm_hour = hour;
        tm_loc.tm_min = min;
        tm_loc.tm_sec = sec;
        time_t tick = mktime(&tm_loc);
        return static_cast<int64_t>(tick) * 1000 + usec;
    }

    int64_t convert_to_datetime(int64_t timestamp)
    {
        int64_t timest = timestamp / 1000;
        int usec = timestamp % 1000;
        time_t tick = (time_t)timest;
        struct tm tm_loc = *localtime(&tick);
        return (tm_loc.tm_year + 1900) * 10000000000000L + (tm_loc.tm_mon + 1) * 100000000000L + tm_loc.tm_mday * 1000000000L + tm_loc.tm_hour * 10000000L + tm_loc.tm_min * 100000L + tm_loc.tm_sec * 1000L + usec;
    }

    int64_t get_datetime_ms()
    {
#ifdef WIN32
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        return wtm.wYear * 10000000000000L
            + wtm.wMonth * 100000000000L
            + wtm.wDay * 1000000000L
            + wtm.wHour * 10000000L +
            wtm.wMinute * 100000L +
            wtm.wSecond * 1000 +
            wtm.wMilliseconds;
#else
        struct timeb raw_time;
        ftime(&raw_time);
        struct tm tm_loc;
        localtime_r(&raw_time.time, &tm_loc);
        return (tm_loc.tm_year + 1900) * 10000000000000L
            + (tm_loc.tm_mon + 1) * 100000000000L
            + (tm_loc.tm_mday) * 1000000000L
            + tm_loc.tm_hour * 10000000L
            + tm_loc.tm_min * 100000L
            + tm_loc.tm_sec * 1000
            + raw_time.millitm;
#endif
    }

    int time_to_int(char* t)
    {
        char tmp[9];
        int count = 0;
        for (int i = 0; i < 8; ++i)
        {
            if (t[i] == ':')
            {
                continue;
            }
            tmp[count++] = t[i];
        }
        tmp[count] = 0;
        return atoi(tmp);
    }

    void trim_memcpy(char* dest, int dest_size, const char* src, int src_size)
    {
        if (dest == NULL || src == NULL)
        {
            return;
        }
        int m = 0;
        for (m = src_size - 1; m >= 0; --m)
        {
            if (src[m] != ' ')
            {
                break;
            }
        }
        if (m < 0)
        {
            dest[0] = 0;
            return;
        }
        int ava_len = dest_size - 1;
        int i;
        for (i = 0; i < ava_len; ++i)
        {
            if (i > m)
            {
                dest[i] = 0;
                return;
            }
            dest[i] = src[i];
        }
        dest[i] = 0;
    }

    int to_2power(int n)
    {
        int h = n;
        int count = 0;
        while (true)
        {
            h >>= 1;
            if (h == 0)
            {
                ++count;
                break;
            }
            ++count;
        }
        return 1 << count;
    }

    void trim(char* str)
    {
        int len = 0;
        if ((str == NULL) || ((len = strlen(str)) == 0))
        {
            return;
        }

        int h;
        for (int i = 0; i < len; ++i)
        {
            if (str[i] != ' ')
            {
                for (h = len - 1; h > i; --h)
                {
                    if (str[h] != ' ')
                    {
                        break;
                    }
                }
                memmove(str, str + i, h - i + 1);
                str[h - i + 1] = 0;
                return;
            }
        }

        str[0] = 0;
    }

    void trim_left(char* str, char c)
    {
        int len = 0;
        if ((str == NULL) || ((len = strlen(str)) == 0))
        {
            return;
        }

        for (int i = 0; i < len; ++i)
        {
            if (str[i] != c)
            {
                memmove(str, str + i, len - i);
                str[len - i] = 0;
                return;
            }
        }

        str[0] = 0;
    }


    std::vector<std::string> str_split(const char* str, const char* split_flag)
    {
        if (nullptr == str || NULL == str || strlen(str) <= 0)
        {
            return std::vector<std::string>();
        }

        std::vector<std::string> v_split;
        if (nullptr == split_flag || NULL == split_flag || strlen(split_flag) <= 0)
        {
            v_split.push_back(str);
        }
        else
        {
            std::string s = std::string(str);
            std::string::size_type pos2 = s.find(split_flag);
            std::string::size_type pos1 = 0;
            while (std::string::npos != pos2)
            {
                v_split.push_back(s.substr(pos1, pos2 - pos1));
                pos1 = pos2 + strlen(split_flag);
                pos2 = s.find(split_flag, pos1);
            }

            if (pos1 != s.length())
            {
                v_split.push_back(s.substr(pos1));
            }
        }

        return v_split;
    }

    int double_compare(double a, double b)
    {
        double c = a - b;
        if (c >= -FLT_EPSILON && c <= FLT_EPSILON)
        {
            return 0;
        }
        else if (c > FLT_EPSILON)
        {
            return 1;
        }
        return -1;
    }

    double round(const double value, int digits)
    {
        if (double_compare(value, 0.0) <= 0)
        {
            return value;
        }
        int div = pow(10, digits);
        return floor(value * div + 0.5) / div;
    }

    const char* getProjectName()
    {
        return PROJECT_NAME;
    }

    const char* getVersion()
    {
        return P_VERSION;
    }

    const char* getBuildTime()
    {
        return RELEASE_DATETIME;
    }

    const char* getGitRevision()
    {
        return GIT_REVISION;
    }
}
