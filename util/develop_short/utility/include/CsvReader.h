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

        // ��ȡ������
        void GetFieldHeaders(TRecord& header);

        // ��ȡ��һ�м�¼
        bool GetNextRecord(TRecord& record);

        // ��ȡȫ����¼
        bool GetRecords(TRecords& records);

        // ��ȡ��һ������Ϣ
        std::string GetLastErr();

        // �����ļ�
        void SetFileName(std::string filename);
        int GetFieldCount() { return m_filedCount; }

        std::string Trim(std::string& str);
    protected:
        bool OpenFile();    // ���ļ�
        void CloseFile();   // �ر��ļ�

        bool ParseLine(std::string& line, TRecord& record);
        bool ParseLine(char* line, TRecord& record);

    protected:
        std::string m_errmsg;        // ������Ϣ
        std::string m_filename;      // �ļ���
        char m_seperator;     // �ָ���
        bool m_have_header;     // �Ƿ��һ��Ϊ����
        TRecord m_header;       // ������

        int m_filedCount;
        std::ifstream m_file;
        bool m_bTrim;
    };
}

#endif
