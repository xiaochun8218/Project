/*
 * @Descripttion:
 * @version: 1.0.0.0
 * @Author: wangfuchun
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: wangfuchun
 * @LastEditTime: 2020-07-07 21:48:46
 */
#pragma once

#include "function.h"

#define ACTIVE_STEP_SIZE 1000
#define ACTIVE_DEFAULT_SIZE 1000

namespace datacenter
{
    template <typename T>
    class em_point_array
    {

    public:
        em_point_array()
        {
            m_array = NULL;
            m_curIndex = 0;
            m_totalSize = 0;
            m_step = ACTIVE_STEP_SIZE;
        }

        ~em_point_array()
        {
            SAFEFREE(m_array);
        }

        inline int64_t Initialize(int64_t init_size)
        {
            return Initialize(init_size, NULL, ACTIVE_STEP_SIZE);
        }

        inline int64_t Initialize(int64_t size, T defaultT, int step = ACTIVE_STEP_SIZE)
        {
            m_totalSize = size;
            if (step > 0)
            {
                m_step = step;
            }
            else
            {
                m_step = ACTIVE_STEP_SIZE;
            }

            if (m_totalSize <= 0)
            {
                m_totalSize = ACTIVE_DEFAULT_SIZE;
            }

            int64_t  mallocsize = m_totalSize * sizeof(T);
            m_array = (T*)malloc(mallocsize);
            if (NULL == m_array)
            {
                mallocsize = 0;
                return 0;
            }

            memset(m_array, 0, mallocsize);
            m_default = defaultT;
            return mallocsize;
        }

        inline int64_t GetCurIndex()
        {
            return m_curIndex;
        }

        inline int64_t push_back(T t)
        {
            if (NULL == m_array)
            {
                return -1;
            }
            if (!check())
            {
                return -1;
            }

            m_array[m_curIndex] = t;
            int64_t pos = m_curIndex;
            ++m_curIndex;
            return pos;
        }

        inline T at(int64_t i)
        {
            if (NULL == m_array)
            {
                return m_default;
            }

            if (!check())
            {
                return m_default;
            }

            return m_array[i];
        }

        //inline T operator[](int64_t i)
        //{
        //    return at(i);
        //}

        inline T& operator[](int64_t i)
        {
            if (NULL == m_array)
            {
                return m_default;
            }

            if (!check())
            {
                return m_default;
            }

            return m_array[i];
        }

        inline int64_t size()
        {
            return m_curIndex;
        }

        inline void clear()
        {
            m_curIndex = 0;
        }
    private:

        inline bool check()
        {
            if (m_curIndex >= m_totalSize)
            {
                int oldsize = m_totalSize;
                T* p = (T*)realloc(m_array, (oldsize + m_step) * sizeof(T));
                if (NULL == p)
                {
                    return false;
                }

                m_array = p;
                m_totalSize = oldsize + m_step;
            }

            return true;
        }

    private:
        T* m_array;
        T m_default;
        int64_t m_totalSize;
        int64_t m_curIndex;
        int m_step;
    };
}