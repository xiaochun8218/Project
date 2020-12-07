#pragma once

#include <atomic>
#include <string>
#include "hiredis.h"
#include "log.h"


/** redis �������� */
#define  COMMAND_SUBSCRIBE      "SUBSCRIBE"

/** redis �������� */
#define  COMMAND_PUBLISH        "PUBLISH"

/** redis list push ���� */
#define  COMMAND_LPUSH          "LPUSH"

namespace datacenter
{
    class redisclient
    {
    public:
        redisclient(CLog* clog, const char* addr, int port, const char* pwd = NULL);
        ~redisclient();
        /*!
         *
         * @brief               redis �ͻ������ӷ����.
         * @return              �Ƿ�������
         * @retval bool
         *  - true              ���ӽ����ɹ�
         *  - false             ���ӽ���ʧ��
         *
         */
        bool connect();

        /*!
         *
         * @brief               redis �ͻ��˶Ͽ������˵�����.
         * @return              �����Ƿ�ɹ��Ͽ�
         * @retval bool
         *  - true              �Ͽ�����
         *  - false             �Ͽ�����ʧ��
         * @note   matters needing attention
         * @see    other functions
         *
         */
        bool disconnect();

        /*!
         *
         * @brief               ͬ��������Ϣ.
         * @param[in] listId    Ƶ��
         * @param[in] value     ��Ϣ
         * @return              �Ƿ��ĳɹ�
         * @retval bool
         *  - true              �ɹ�
         *  - false             ʧ��
         *
         */
        bool subscribe(const std::string& channel_name);

        /*!
         *
         * @brief               ���Ԫ��.
         * @param[in] listId    redis key
         * @param[in] value     valueֵ
         * @return              �Ƿ���ӳɹ�
         * @retval bool
         *  - true              �ɹ�
         *  - false             ʧ��
         *
         */
        bool lpush(const std::string& listId, const std::string& value);

        /*!
         *
         * @brief               ���Ԫ��.
         * @param[in] listId    redis key
         * @param[in] value     valueֵ
         * @return              �Ƿ���ӳɹ�
         * @retval bool
         *  - true              �ɹ�
         *  - false             ʧ��
         *
         */
        bool rpush(const std::string& listId, const std::string& value);

        /*!
         *
         * @brief               ͬ��������Ϣ.
         * @param[in] listId    Ƶ��
         * @param[in] value     ��Ϣ
         * @return              �Ƿ񷢲��ɹ�
         * @retval bool
         *  - true              �ɹ�
         *  - false             ʧ��
         *
         */
        bool publish(const std::string& channel_name, const std::string& message);

        /*!
         *
         * @brief                   hmsest.
         * @param[in] key           redis key
         * @param[in] hashId        hash id
         * @param[in] value         hash ֵ
         * @return                  �Ƿ�д��ɹ�
         * @retval bool
         *  - true                  �ɹ�
         *  - false                 ʧ��
         *
         */
        bool hmset(const std::string& key, const std::string& hashId, const std::string value);

        /*!
         *
         * @brief                   hmget
         * @param[in] key           redis key
         * @param[in] hashId        hash id
         * @return                  valueֵ
         * @retval std::string
         *
         */
        std::string hmget(const std::string& key, const std::string& hashId);

    private:
        bool connectWithoutLock();
    private:
        std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
        redisContext* m_redisContext;
        char m_addr[32];
        int m_port;
        char m_pwd[128];
        CLog* m_log;
        bool m_connected;

    };
}