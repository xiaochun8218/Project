#ifndef __CSV_READER_H__
#define __CSV_READER_H__

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

namespace datacenter
{
    typedef std::vector <std::string> TRecord;
    typedef std::vector <TRecord> TRecords;
    class CCsvReader
    {
    public:
        CCsvReader(int trim = 1);
        CCsvReader(std::string filename, int trim = 1);
        CCsvReader(std::string filename, char seperator, int trim = 1);
        CCsvReader(std::string filename, char seperator, bool bhave_header_row, int trim = 1);
        CCsvReader(std::string filename, bool bhave_header_row, int trim = 1);
        virtual ~CCsvReader();

        // 获取标题行
        void GetFieldHeaders(TRecord& header);

        // 获取下一行记录
        bool GetNextRecord(TRecord& record);

        // 获取全部记录
        bool GetRecords(TRecords& records);

        // 获取上一错误信息
        std::string GetLastErr();

        // 设置文件
        void SetFileName(std::string filename);
        int GetFieldCount() { return m_filedCount; }

        std::string Trim(std::string& str);
    protected:
        bool OpenFile();    // 打开文件
        void CloseFile();   // 关闭文件

        bool ParseLine(std::string& line, TRecord& record);
        bool ParseLine(char* line, TRecord& record);

    protected:
        std::string m_errmsg;        // 错误信息
        std::string m_filename;      // 文件名
        char m_seperator;     // 分隔符
        bool m_have_header;     // 是否第一行为标题
        TRecord m_header;       // 标题行

        int m_filedCount;
        std::ifstream m_file;
        bool m_bTrim;
    };
}

#endif
