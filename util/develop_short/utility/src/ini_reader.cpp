#include "ini_reader.h"
#include <string>

namespace datacenter
{
    CIniReader::CIniReader(const char* file)
    {
        load(file);
    }

    CIniReader::~CIniReader()
    {
        for (ST_SECTION* it : m_Config)
        {
            delete it;
        }

        m_Config.clear();
    }

    bool CIniReader::load(const char* file)
    {
        std::ifstream fstream(file);
        if (!fstream.is_open())
        {
            return false;
        }

        std::string line;
        std::string section = "";
        ST_SECTION* pSection = NULL;
        std::string tmpdata;
        while (getline(fstream, tmpdata))
        {
            line = string_strip(tmpdata);
            if (line.size() == 0)
            {
                continue;
            }
            if (isComment(line))
            {
                continue;
            }
            else if (isSection(line))
            {
                section = string_strip(std::string(line, 1, line.size() - 2));
                pSection = new ST_SECTION;
                pSection->section = section;
                m_Config.push_back(pSection);
                section += "-";
                continue;
            }
            else if (isKeyValue(line) && section != "")
            {
                std::pair<std::string, std::string> keyValue = splitKeyValue(line);
                m_KeyValue[section + keyValue.first] = keyValue.second;
                if (pSection != NULL)
                {
                    pSection->key_values.push_back(keyValue);
                }
            }
        }
        fstream.close();
        return true;
    }

    const std::vector<ST_SECTION*>& CIniReader::GetConfig()
    {
        return m_Config;
    }

    bool CIniReader::isComment(const std::string& line)
    {
        if (line[0] == '#' || line[0] == ';')
        {
            return true;
        }
        return false;
    }

    bool CIniReader::isSection(const std::string& line)
    {
        return line[0] == '[' && line[line.size() - 1] == ']';
    }

    bool CIniReader::isKeyValue(const std::string& line)
    {
        return line.find('=') != std::string::npos;
    }

    std::string CIniReader::string_strip(const std::string& value)
    {
        if (!value.size())
            return value;

        size_t startPos = value.find_first_not_of(" \t\r\n");
        size_t endPos = value.find_last_not_of(" \t\r\n");

        if (startPos == std::string::npos)
            return value;

        return std::string(value, startPos, endPos - startPos + 1);
    }

    std::pair<std::string, std::string> CIniReader::splitKeyValue(const std::string& line)
    {
        size_t equals = line.find('=');
        std::string key = string_strip(std::string(line, 0, equals));
        std::string value = string_strip(std::string(line, equals + 1, std::string::npos));
        return std::pair<std::string, std::string>(key, value);
    }

    const char* CIniReader::read_string(const char* section, const char* key, const char* defaultValue)
    {
        if (section == NULL || key == NULL)
        {
            return defaultValue;
        }
        std::string mapkey = section;
        mapkey += "-";
        mapkey += key;
        std::map<std::string, std::string>::iterator it = m_KeyValue.find(mapkey);
        if (it != m_KeyValue.end())
        {
            return it->second.c_str();
        }

        return defaultValue;
    }

    char CIniReader::read_char(const char* section, const char* key, char defaultValue)
    {
        const char* str = read_string(section, key);
        if (NULL == str || strlen(str) <= 0) {
            return defaultValue;
        }

        return str[0];
    }

    int CIniReader::read_int(const char* section, const char* key, int defaultValue)
    {
        const char* str = read_string(section, key);
        if (NULL == str || strlen(str) <= 0) {
            return defaultValue;
        }

        return atoi(str);
    }

    long CIniReader::read_long(const char* section, const char* key, long defaultValue)
    {
        const char* str = read_string(section, key);
        if (NULL == str || strlen(str) <= 0) {
            return defaultValue;
        }

        return atol(str);
    }

    long long CIniReader::read_long_long(const char* section, const char* key, long long defaultValue)
    {
        const char* str = read_string(section, key);
        if (NULL == str || strlen(str) <= 0) {
            return defaultValue;
        }

        return atoll(str);
    }

    double CIniReader::read_double(const char* section, const char* key, double defaultValue)
    {
        const char* str = read_string(section, key);
        if (NULL == str || strlen(str) <= 0) {
            return defaultValue;
        }

        return atof(str);
    }
}
