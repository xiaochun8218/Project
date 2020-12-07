#include "CsvReader.h"

namespace datacenter
{
    CCsvReader::CCsvReader(int trim)
        : m_filename(""), m_seperator(','), m_have_header(false), m_bTrim(trim)
    {

    }

    CCsvReader::CCsvReader(std::string filename, int trim)
        : m_filename(filename), m_seperator(','), m_have_header(false), m_bTrim(trim)
    {
        OpenFile();
    }

    CCsvReader::CCsvReader(std::string filename, char seperator, int trim)
        : m_filename(filename), m_seperator(seperator), m_have_header(false), m_bTrim(trim)
    {
        OpenFile();
    }

    CCsvReader::CCsvReader(std::string filename, char seperator, bool bhave_header_row, int trim)
        : m_filename(filename), m_seperator(seperator), m_have_header(bhave_header_row), m_bTrim(trim)
    {
        OpenFile();
    }

    CCsvReader::CCsvReader(std::string filename, bool bhave_header_row, int trim)
        : m_filename(filename), m_seperator(','), m_have_header(bhave_header_row), m_bTrim(trim)
    {
        OpenFile();
    }

    CCsvReader::~CCsvReader()
    {
        CloseFile();
    }

    bool CCsvReader::OpenFile()
    {
        m_filedCount = 0;
        m_errmsg = "";
        if (m_filename == "")
        {
            throw "文件为空";
        }
        m_file.open(m_filename.c_str());
        if (!m_file)
        {
            //throw "文件打开失败:" + m_filename;
            //m_errmsg = "文件打开失败";
            return false;
        }
        // 如果有标题行，取标题行
        if (m_have_header)
        {
            GetNextRecord(m_header);
            m_filedCount = m_header.size();
        }
        return true;
    }

    // 关闭文件
    void CCsvReader::CloseFile()
    {
        if (m_file.is_open())
        {
            m_file.close();
        }
    }

    void CCsvReader::GetFieldHeaders(TRecord& header)
    {
        header = m_header;
    }

    // 设置文件名
    void CCsvReader::SetFileName(std::string filename)
    {
        m_filename = filename;
        CloseFile();
        OpenFile();
    }

    // 获取下一条记录
    bool CCsvReader::GetNextRecord(TRecord& record)
    {
        char stext[2048] = { 0 };
        std::string line;
        if (!m_file)
        {
            return false;
        }
        if (m_file.eof())
        {
            return false;
        }

        //getline(m_file, line);
        //ParseLine(line, record);

        m_file.getline(stext, sizeof(stext));
        ParseLine(stext, record);

        return true;
    }

    // 解析行
    bool CCsvReader::ParseLine(char* line, TRecord& record)
    {
        record.clear();
        char* pend = NULL;
        char* pstart = line;
        char str[1024];
        std::string tstr;
        while ((pend = strchr(pstart, m_seperator)) != NULL)
        {
            if (pstart == pend)
            {
                record.push_back("");
            }
            else
            {
                memset(str, 0, sizeof(str));
                strncpy(str, pstart, pend - pstart);
                tstr = str;
                if (m_bTrim)
                {
                    record.push_back(Trim(tstr));
                }
                else
                {
                    record.push_back(tstr);
                }
            }
            pstart = pend + 1;
        }
        tstr = pstart;
        if (m_bTrim)
        {
            record.push_back(Trim(tstr));
        }
        else
        {
            record.push_back(tstr);
        }
        return true;
    }

    // 解析行
    bool CCsvReader::ParseLine(std::string& line, TRecord& record)
    {
        record.clear();
        /*
            int pre_index = 0, index = 0, len = 0;
            while( (index = line.find_first_of(m_seperator, pre_index)) != -1 )
            {
                if( (len = index-pre_index)!=0 )
                    record.push_back(line.substr(pre_index, len));
                else
                    record.push_back("");
                pre_index = index+1;
            }
            record.push_back(line.substr(pre_index)); */

        long index = -1, last_search_position = 0;
        while ((index = line.find(m_seperator, last_search_position)) != -1)
        {
            if (index == last_search_position)
                record.push_back("");
            else
            {
                std::string tstr = line.substr(last_search_position, index - last_search_position);
                if (m_bTrim)
                {
                    record.push_back(Trim(tstr));
                }
                else
                {
                    record.push_back(tstr);
                }
            }
            last_search_position = index + 1;
        }

        std::string last_one = line.substr(last_search_position);
        if (m_bTrim)
        {
            record.push_back(last_one.empty() ? "" : Trim(last_one));
        }
        else
        {
            record.push_back(last_one.empty() ? "" : last_one);
        }

        return true;
    }

    // 获取全部记录
    bool CCsvReader::GetRecords(TRecords& records)
    {
        TRecord record;
        records.clear();
        while (GetNextRecord(record))
        {
            records.push_back(record);
        }

        return true;
    }

    std::string CCsvReader::Trim(std::string& str)
    {
        size_t begin = str.find_first_not_of(" ");
        size_t end = str.find_last_not_of(" ");
        size_t end1 = str.find_last_not_of("\r");
        size_t end2 = str.find_last_not_of("\n");
        if (begin == std::string::npos || end == std::string::npos || end1 == std::string::npos || end2 == std::string::npos)
        {
            return "";
        }
        if (end > end1)
        {
            end = end1;
        }
        if (end > end2)
        {
            end = end2;
        }
        return str.substr(begin, end + 1);
    }
}