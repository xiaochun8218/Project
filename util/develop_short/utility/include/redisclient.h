#pragma once

#include <atomic>
#include <string>
#include "hiredis.h"
#include "log.h"


/** redis 订阅命令 */
#define  COMMAND_SUBSCRIBE      "SUBSCRIBE"

/** redis 发布命令 */
#define  COMMAND_PUBLISH        "PUBLISH"

/** redis list push 命令 */
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
         * @brief               redis 客户端连接服务端.
         * @return              是否建立连接
         * @retval bool
         *  - true              连接建立成功
         *  - false             连接建立失败
         *
         */
        bool connect();

        /*!
         *
         * @brief               redis 客户端断开与服务端的连接.
         * @return              连接是否成功断开
         * @retval bool
         *  - true              断开连接
         *  - false             断开连接失败
         * @note   matters needing attention
         * @see    other functions
         *
         */
        bool disconnect();

        /*!
         *
         * @brief               同步订阅消息.
         * @param[in] listId    频道
         * @param[in] value     消息
         * @return              是否订阅成功
         * @retval bool
         *  - true              成功
         *  - false             失败
         *
         */
        bool subscribe(const std::string& channel_name);

        /*!
         *
         * @brief               添加元素.
         * @param[in] listId    redis key
         * @param[in] value     value值
         * @return              是否添加成功
         * @retval bool
         *  - true              成功
         *  - false             失败
         *
         */
        bool lpush(const std::string& listId, const std::string& value);

        /*!
         *
         * @brief               添加元素.
         * @param[in] listId    redis key
         * @param[in] value     value值
         * @return              是否添加成功
         * @retval bool
         *  - true              成功
         *  - false             失败
         *
         */
        bool rpush(const std::string& listId, const std::string& value);

        /*!
         *
         * @brief               同步发布消息.
         * @param[in] listId    频道
         * @param[in] value     消息
         * @return              是否发布成功
         * @retval bool
         *  - true              成功
         *  - false             失败
         *
         */
        bool publish(const std::string& channel_name, const std::string& message);

        /*!
         *
         * @brief                   hmsest.
         * @param[in] key           redis key
         * @param[in] hashId        hash id
         * @param[in] value         hash 值
         * @return                  是否写入成功
         * @retval bool
         *  - true                  成功
         *  - false                 失败
         *
         */
        bool hmset(const std::string& key, const std::string& hashId, const std::string value);

        /*!
         *
         * @brief                   hmget
         * @param[in] key           redis key
         * @param[in] hashId        hash id
         * @return                  value值
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