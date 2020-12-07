/*
 * @Descripttion:
 * @version: 1.0.0.0
 * @Author: wangfuchun
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: wangfuchun
 * @LastEditTime: 2020-07-07 21:48:58
 */
#pragma once

#include <cstddef>
#include <iostream>
#include "function.h"

#define ACTIVE_MALLOC_SIZE 1000
#define ACTIVE_DEFAULT_SIZE 1000

namespace datacenter
{
    template <typename T>
    class em_array
    {
    public:
        em_array(void)
        {
            m_array = NULL;
            m_totalSize = 0L;
            m_offset = 0;
            m_initSize = 0;
            m_step = ACTIVE_MALLOC_SIZE;
        }

        ~em_array()
        {
            if (m_array != NULL)
            {
                for (int64_t i = 0; i < m_totalSize; ++i)
                {
                    SAFEFREE(m_array[i]);
                    if (i > 0)
                    {
                        i += m_step;
                        --i;
                    }
                    else
                    {
                        i += m_initSize;
                        --i;
                    }
                }

                free(m_array);
                m_array = NULL;
            }
        }

        inline int64_t Initialize(int64_t size, int offset)
        {
            return Initialize(size, ACTIVE_MALLOC_SIZE, offset);
        }

        inline int64_t Initialize(int64_t size, int step, int offset)
        {
            if (step > 0)
            {
                m_step = step;
            }
            else
            {
                m_step = ACTIVE_MALLOC_SIZE;
            }

            m_totalSize = size;
            m_offset = offset;
            if (m_totalSize <= 0)
            {
                m_totalSize = ACTIVE_DEFAULT_SIZE;
            }

            m_initSize = m_totalSize;
            int64_t mallocsize = m_totalSize * sizeof(T);
            T* p = (T*)malloc(mallocsize);
            if (NULL == p)
            {
                mallocsize = 0;
                return false;
            }

            memset((void*)p, 0, mallocsize);
            m_array = (T**)malloc(sizeof(T*) * m_totalSize);
            if (NULL == m_array)
            {
                return false;
            }

            for (int64_t i = 0; i < m_totalSize; ++i)
            {
                m_array[i] = &(p[i]);
            }

            return true;
        }

        inline T* operator[](int i)
        {
            if (NULL == m_array)
            {
                return NULL;
            }

            int64_t tpos = i - m_offset;
            if (tpos < 0)
            {
                return NULL;
            }
            else if (tpos >= m_totalSize)
            {
                T* p = (T*)malloc(m_step * sizeof(T));
                if (NULL == p)
                {
                    return NULL;
                }

                memset((void*)p, 0, m_step * sizeof(T));
                T** p2 = (T**)realloc(m_array, (m_totalSize + m_step) * sizeof(T*));
                if (NULL == p2)
                {
                    delete p;
                    return NULL;
                }
                m_array = p2;
                int64_t i = m_totalSize;
                int64_t n = 0;
                m_totalSize += m_step;
                for (i, n; i < m_totalSize; ++i, ++n)
                {
                    m_array[i] = &(p[n]);
                }
            }
            return m_array[tpos];
        }

        void clear()
        {
            if (m_array != NULL)
            {
                for (int64_t i = 0; i < m_totalSize; ++i)
                {
                    SAFEFREE(m_array[i]);
                    if (i > 0)
                    {
                        i += m_step;
                        --i;
                    }
                    else
                    {
                        i += m_initSize;
                        --i;
                    }
                }
            }
        }
    private:
        T** m_array;
        int64_t m_totalSize;
        int m_offset;
        int m_initSize;
        int m_step;
    };
}