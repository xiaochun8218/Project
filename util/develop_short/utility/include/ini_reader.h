/*****************************************************************************
*  DDBS INI File Reader											             *
*  Copyright (C) 2019                                                        *
*                                                                            *
*  @file     ini_reader.h                                                    *
*  @brief    读取INI配置文件                                                 *
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
#ifndef __DDBS_INI_READER_H__
#define __DDBS_INI_READER_H__
#include <map>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <vector>

namespace datacenter
{
    typedef struct
    {
        std::string section;
        std::vector< std::pair<std::string, std::string> > key_values;
    }ST_SECTION;

    class CIniReader
    {
    public:
        CIniReader(const char* file);
        ~CIniReader();

        const char* read_string(const char* section, const char* key, const char* defaultValue = NULL);
        char read_char(const char* section, const char* key, char defaultValue = 0);
        int read_int(const char* section, const char* key, int defaultValue = 0);
        long read_long(const char* section, const char* key, long defaultValue = 0);
        long long read_long_long(const char* section, const char* key, long long defaultValue = 0L);
        double read_double(const char* section, const char* key, double defaultValue = 0.0);
        const std::vector<ST_SECTION*>& GetConfig();
    private:
        bool load(const char* file);
        bool isComment(const std::string& line);
        bool isSection(const std::string& line);
        bool isKeyValue(const std::string& line);
        std::string string_strip(const std::string& value);
        std::pair<std::string, std::string> splitKeyValue(const std::string& line);
    private:
        std::map<std::string, std::string> m_KeyValue;
        std::vector<ST_SECTION*> m_Config;
    };
}

#endif