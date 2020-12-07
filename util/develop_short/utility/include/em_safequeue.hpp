/*
 * @Descripttion:
 * @version: 1.0.0.0
 * @Author: wangfuchun
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: wangfuchun
 * @LastEditTime: 2020-07-12 18:09:23
 */
#pragma once
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <atomic>

namespace datacenter
{
    template <typename T>
    class em_safequeue
    {
    protected:
        mutable std::mutex m_mut; //必须是mutable，因为empty是const方法，但是要锁mut，锁操作就是改变操作
        std::queue<T> m_queue;
        std::condition_variable data_cond;
        std::atomic<bool> m_bTermination;

    public:
        em_safequeue()
        {
            m_bTermination = false;
        }

        em_safequeue(const em_safequeue<T>& other)
        {
            m_bTermination = false;
            std::lock_guard<std::mutex> lk(other.m_mut);
            m_queue = other.m_queue;
        }

        ~em_safequeue()
        {
            m_bTermination = true;
            data_cond.notify_all();
        }

        void push(T value)
        {
            if (m_bTermination)
            {
                return;
            }

            std::lock_guard<std::mutex> lk(m_mut);
            m_queue.push(value);
            data_cond.notify_one();
        }

        bool wait_and_pop(T& value)
        {
            std::unique_lock<std::mutex> lk(m_mut);
            data_cond.wait(lk, [this] { return (!m_queue.empty() || m_bTermination); });
            if (!m_queue.empty())
            {
                value = std::move(m_queue.front());
                m_queue.pop();
                return true;
            }

            return false;
        }

        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> lk(m_mut);
            data_cond.wait(lk, [this] { return (!m_queue.empty() || m_bTermination); });
            //不为空则出队
            if (!m_queue.empty())
            {
                std::shared_ptr<T> res(std::make_shared<T>(std::move(m_queue.front())));
                m_queue.pop();
                return res;
            }

            return NULL;
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lk(m_mut);
            return m_queue.empty();
        }

        int size() const
        {
            std::lock_guard<std::mutex> lk(m_mut);
            return m_queue.size();
        }

        /// <summary>
        /// 设置队列为退出状态。在退出状态下，忽略入队，可以执行出队，但当队列为空时，wait_and_pop不会阻塞。
        /// </summary>
        void termination()
        {
            std::lock_guard<std::mutex> lk(m_mut);
            m_bTermination = true;
            data_cond.notify_all();
        }

        /// <summary>
        /// 是否退出状态
        /// </summary>
        /// <returns></returns>
        bool is_termination()
        {
            return m_bTermination;
        }

    };
}