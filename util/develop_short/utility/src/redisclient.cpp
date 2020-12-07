#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "redisclient.h"
#include "function.h"


namespace datacenter
{
    redisclient::redisclient(CLog* clog, const char* addr, int port, const char* pwd)
    {
        m_connected = false;
        m_redisContext = NULL;
        memset(&m_addr, 0, sizeof(m_addr));
        if (addr)
        {
            strncpy(m_addr, addr, sizeof(m_addr) - 1);
        }

        memset(&m_pwd, 0, sizeof(m_pwd));
        if (pwd)
        {
            strncpy(m_pwd, pwd, sizeof(m_pwd) - 1);
        }

        m_port = port;
        m_log = clog;
    }

    redisclient::~redisclient()
    {
        disconnect();
        m_lock.clear();
    }

    bool redisclient::connect()
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        bool result = this->connectWithoutLock();
        m_lock.clear();
        return result;
    }

    bool redisclient::connectWithoutLock()
    {
        bool result;
        if (m_port > 0 && strlen(m_addr))
        {
            redisOptions options = { 0 };
            REDIS_OPTIONS_SET_TCP(&options, m_addr, m_port);
            struct timeval tv = { 2,0 };
            options.timeout = &tv;
            m_redisContext = redisConnectWithOptions(&options);
            if (m_redisContext->err)
            {
                result = false;
                LOG_MSG_ERROR(m_log, "redis connect[%s:%d] failed,err:%s", m_addr, m_port, m_redisContext->errstr);
            }
            else
            {
                if (strlen(m_pwd) > 0)
                {
                    redisReply* reply = (redisReply*)redisCommand(m_redisContext, "AUTH %s", m_pwd);
                    if (NULL == reply)
                    {
                        result = false;
                        LOG_MSG_ERROR(m_log, "auth command failed: pwd:%s", m_pwd);
                    }
                    else
                    {
                        m_connected = true;
                        result = true;
                    }
                }
                else
                {
                    m_connected = true;
                    result = true;
                }
            }
        }
        else
        {
            result = false;
            m_redisContext = NULL;
        }

        return result;
    }

    bool redisclient::disconnect()
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        bool result;
        if (m_redisContext)
        {
            redisFree(m_redisContext);
            m_connected = false;
            m_redisContext = NULL;
            result = true;
        }
        else
        {
            result = false;
        }

        m_lock.clear();
        return result;
    }

    bool redisclient::subscribe(const std::string& channel_name)
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        bool result;
        if (!m_connected)
        {
            connectWithoutLock();
        }

        if (m_redisContext)
        {
            redisReply* reply = (redisReply*)redisCommand(m_redisContext, "SUBSCRIBE %s", channel_name.c_str());
            if (NULL == reply)
            {
                result = false;
                if (m_redisContext->err)
                {
                    LOG_MSG_ERROR(m_log, "SUBSCRIBE command failed: channel_name:%s,err code:%d,err msg:%s", channel_name.c_str(), m_redisContext->err, m_redisContext->errstr);
                }
                else
                {
                    LOG_MSG_ERROR(m_log, "SUBSCRIBE command failed: channel_name:%s", channel_name.c_str());
                }

                redisFree(m_redisContext);
                connectWithoutLock();
            }
            else
            {
                freeReplyObject(reply);
                result = true;
            }
        }
        else
        {
            result = false;
        }

        m_lock.clear();
        return result;
    }

    bool redisclient::publish(const std::string& channel_name, const std::string& message)
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        bool result;
        if (!m_connected)
        {
            connectWithoutLock();
        }

        if (m_redisContext)
        {
            redisReply* reply = (redisReply*)redisCommand(m_redisContext, "PUBLISH %s %s", channel_name.c_str(), message.c_str());
            if (NULL == reply)
            {
                if (m_redisContext->err)
                {
                    LOG_MSG_ERROR(m_log, "PUBLISH command failed: channel_name:%s,message:%s,err code:%d,err msg:%s", channel_name.c_str(), message.c_str(), m_redisContext->err, m_redisContext->errstr);
                }
                else
                {
                    LOG_MSG_ERROR(m_log, "PUBLISH command failed: channel_name:%s,message:%s", channel_name.c_str(), message.c_str());
                }

                redisFree(m_redisContext);
                connectWithoutLock();
                result = false;
            }
            else
            {
                freeReplyObject(reply);
                result = true;
            }
        }
        else
        {
            result = false;
        }

        m_lock.clear();
        return result;
    }

    bool redisclient::lpush(const std::string& listId, const std::string& value)
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        bool result;
        if (!m_connected)
        {
            connectWithoutLock();
        }

        if (m_redisContext)
        {
            redisReply* reply = (redisReply*)redisCommand(m_redisContext, "LPUSH %s %s", listId.c_str(), value.c_str());
            if (NULL == reply)
            {
                result = false;
                if (m_redisContext->err)
                {
                    LOG_MSG_ERROR(m_log, "LPUSH command failed: channel_name:%s,message:%s,err code:%d,err msg:%s", listId.c_str(), value.c_str(), m_redisContext->err, m_redisContext->errstr);
                }
                else
                {
                    LOG_MSG_ERROR(m_log, "LPUSH command failed: channel_name:%s,message:%s", listId.c_str(), value.c_str());
                }

                redisFree(m_redisContext);
                connectWithoutLock();
            }
            else if (REDIS_REPLY_INTEGER == reply->type && reply->integer > 0)
            {
                result = true;
                freeReplyObject(reply);
            }
            else
            {
                result = false;
                LOG_MSG_ERROR(m_log, "LPUSH command failed: channel_name:%s,message:%s", listId.c_str(), value.c_str());
                freeReplyObject(reply);
            }
        }
        else
        {
            result = false;
        }

        m_lock.clear();
        return result;
    }

    bool redisclient::rpush(const std::string& listId, const std::string& value)
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        bool result;
        if (!m_connected)
        {
            connectWithoutLock();
        }

        if (m_redisContext)
        {
            redisReply* reply = (redisReply*)redisCommand(m_redisContext, "RPUSH %s %s", listId.c_str(), value.c_str());
            if (NULL == reply)
            {
                result = false;
                if (m_redisContext->err)
                {
                    LOG_MSG_ERROR(m_log, "LPUSH command failed: channel_name:%s,message:%s,err code:%d,err msg:%s", listId.c_str(), value.c_str(), m_redisContext->err, m_redisContext->errstr);
                }
                else
                {
                    LOG_MSG_ERROR(m_log, "LPUSH command failed: channel_name:%s,message:%s", listId.c_str(), value.c_str());
                }

                redisFree(m_redisContext);
                connectWithoutLock();
            }
            else if (REDIS_REPLY_INTEGER == reply->type && reply->integer > 0)
            {
                result = true;
                freeReplyObject(reply);
            }
            else
            {
                result = false;
                LOG_MSG_ERROR(m_log, "LPUSH command failed: channel_name:%s,message:%s", listId.c_str(), value.c_str());
                freeReplyObject(reply);
            }
        }
        else
        {
            result = false;
        }
        m_lock.clear();
        return result;
    }

    bool redisclient::hmset(const std::string& key, const std::string& hashId, const std::string value)
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        bool result;
        if (!m_connected)
        {
            connectWithoutLock();
        }

        if (m_redisContext)
        {
            redisReply* reply = (redisReply*)redisCommand(m_redisContext, "HMSET %s %s %s", key.c_str(), hashId.c_str(), value.c_str());
            if (NULL == reply)
            {
                result = false;
                if (m_redisContext->err)
                {
                    LOG_MSG_ERROR(m_log, "HMSET command failed: key:%s,hashId:%s,message:%s,err code:%d,err msg:%s", key.c_str(), hashId.c_str(), value.c_str(), m_redisContext->err, m_redisContext->errstr);
                }
                else
                {
                    LOG_MSG_ERROR(m_log, "HMSET command failed:key:%s,hashId:%s,message:%s", key.c_str(), hashId.c_str(), value.c_str());
                }

                redisFree(m_redisContext);
                connectWithoutLock();
            }
            else if (REDIS_REPLY_STATUS == reply->type && strncmp("OK", reply->str, reply->len) == 0)
            {
                result = true;
                freeReplyObject(reply);
            }
            else
            {
                result = false;
                LOG_MSG_ERROR(m_log, "HMSET command failed: key:%s,hashId:%s,message:%s", key.c_str(), hashId.c_str(), value.c_str());
                freeReplyObject(reply);
            }
        }
        else
        {
            result = false;
        }

        m_lock.clear();
        return result;
    }

    std::string redisclient::hmget(const std::string& key, const std::string& hashId)
    {
        while (m_lock.test_and_set())
        {
            msleep(1);
        }

        std::string strvalue;
        if (!m_connected)
        {
            connectWithoutLock();
        }

        if (m_redisContext)
        {
            redisReply* reply = (redisReply*)redisCommand(m_redisContext, "HMGET %s %s", key.c_str(), hashId.c_str());
            if (NULL == reply)
            {
                if (m_redisContext->err)
                {
                    LOG_MSG_ERROR(m_log, "HMGET command failed: key:%s,hashId:%s,err code:%d,err msg:%s", key.c_str(), hashId.c_str(), m_redisContext->err, m_redisContext->errstr);
                }
                else
                {
                    LOG_MSG_ERROR(m_log, "HMGET command failed:key:%s,hashId:%s", key.c_str(), hashId.c_str());
                }

                redisFree(m_redisContext);
                connectWithoutLock();
                strvalue = std::string("");
            }
            else if (REDIS_REPLY_ARRAY == reply->type && reply->element && reply->elements > 0)
            {
                strvalue = (NULL != reply->element[0]->str)
                    ? std::string(reply->element[0]->str)
                    : std::string("");

                freeReplyObject(reply);
            }
            else
            {
                freeReplyObject(reply);
                LOG_MSG_ERROR(m_log, "HMGET command failed: key:%s,hashId:%s", key.c_str(), hashId.c_str());
                strvalue = std::string("");
            }
        }
        else
        {
            strvalue = std::string("");
        }

        m_lock.clear();
        return strvalue;
    }
}
