#include <iostream>
#include <atomic>
#include <string.h>
#include "function.h"
#include "redisclient.h"
#include "em_zskiplist.hpp"
#include "log.h"
#include <climits>
#include "queue/blockingconcurrentqueue.h"
#include "queue/readerwriterqueue.h"

datacenter::CLog* clog;
struct packages
{
    uint32_t data;
};

int redisTest()
{
    datacenter::CLog* clog = new datacenter::CLog;
    clog->SetLogPath("./logs");
    datacenter::redisclient client(clog, "172.30.70.23", 6379);
    client.connect();
    client.hmset("f_test", "f_quote_rt", "12131");

    std::string value = client.hmget("f_test", "stestset1");
    std::string a;
    std::cout << value << std::endl;
    std::cin >> a;
    return 0;
}

int split_test() {
    const char* str = "lev2  0 0 1";
    std::vector<std::string> v = datacenter::str_split(str, " ");
    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
    {
        std::cout << (*it).c_str() << std::endl;
    }

    return 0;
}

void zsetTest()
{
    int test_n = 1000;
    datacenter::em_zskiplist<char*>* sl = new datacenter::em_zskiplist<char*>;
    sl->zslCreate();
    for (int i = 0; i < test_n; i += 2)
    {
        char* sds_data = new char[15];
        memset(sds_data, 0, 15);
        snprintf(sds_data, 14, "data_%05d", i);
        datacenter::zskiplistNode<char*>* zkNode = sl->zslInsert(i % 500, sds_data);
        std::cout << zkNode->data << std::endl;
    }

    int64_t start = datacenter::gettimestamp();
    std::vector<char*> v_sds = sl->getRange(5, 100, 0, 0);

    int64_t end = datacenter::gettimestamp();
    std::cout << v_sds.size() << ",spent:" << end - start << std::endl;

    for (std::vector<char*>::iterator it = v_sds.begin(); it < v_sds.end(); ++it)
    {
        std::cout << (*it) << std::endl;
    }

    delete sl;
}

moodycamel::BlockingConcurrentQueue<packages*>* safe_queue = new moodycamel::BlockingConcurrentQueue<packages*>;

void readerwriterqueue_test()
{
    moodycamel::BlockingReaderWriterQueue<packages*> readerwritequeue;
    std::thread producer = std::thread([&] {
        for (size_t i = 0; i < UINT_MAX; ++i)
        {
            packages* packag = new packages;
            packag->data = i;
            if (!readerwritequeue.enqueue(packag))
            {
                LOG_MSG_ERROR(clog, "enqueue data failed!", packag->data);
                SAFEDELETE(packag);
            }

            if (i % 1000000 == 0)
            {
                std::cout << "-----------statistics:" << i << ",queue size:" << readerwritequeue.size_approx() << "---------" << std::endl;
            }
        }

        });


    std::thread consumer = std::thread([&]
        {
            int l_data = 0;
            packages* packag = nullptr;
            while (true)
            {
                if (readerwritequeue.wait_dequeue_timed(packag, std::chrono::seconds(1)))
                {
                    if (packag->data - l_data != 1)
                    {
                        std::cout << "l_data:" << l_data << ",data:" << packag->data << std::endl;
                    }

                    l_data = packag->data;
                    delete packag;
                    packag = nullptr;
                    if (l_data == UINT_MAX)
                    {
                        break;
                    }
                }
            }

            std::cout << "消费完成!" << std::endl;
        });

    producer.join();
    consumer.join();
}

void* produce_data(void* data)
{
    for (size_t i = 0; i < UINT_MAX; ++i)
    {
        packages* packag = new packages;
        packag->data = i;
        if (!safe_queue->enqueue(packag))
        {
            LOG_MSG_ERROR(clog, "enqueue data failed!", packag->data);
            SAFEDELETE(packag);
        }

        if (i % 1000000 == 0)
        {
            LOG_MSG_DEBUG(clog, "-----------statistics:%u,queue size:%u---------", i, safe_queue->size_approx())
        }
    }

    return 0;
}

void* consumer_data(void* data)
{
    packages* packag;
    int l_data = 0;
    while (1)
    {
        if (safe_queue->wait_dequeue_timed(packag, 1000))
        {
            if (packag->data - l_data != 1)
            {
                clog->Log(datacenter::LOG_LEVEL::LOG_ERROR, datacenter::CONSOLE_PRINT::NO, "l_data:%u,data:%u", l_data, packag->data);
            }

            l_data = packag->data;
        }
    }

    return 0;
}

void current_queue_test()
{
#ifdef __linux__

    datacenter::StartThread(produce_data, nullptr);
    datacenter::StartThread(consumer_data, nullptr);

#endif // __linux__

}

void logTes() {
    datacenter::CLog clog;
    clog.SetLevel(datacenter::LOG_LEVEL::LOG_DEBUG);
    clog.SetLogPath("logs");
    clog.Open("test");
    clog.Log(datacenter::LOG_LEVEL::LOG_DEBUG, datacenter::CONSOLE_PRINT::YES, "test");
}
packages* a;


int share_ptr_test1(const std::shared_ptr<packages>& v)
{
    std::cout << v.use_count() << "->" << v->data << std::endl;
    std::shared_ptr<packages> t = v;

    std::cout << t.use_count() << "->" << t->data << std::endl;
    t->data += 2;
    std::cout << t.use_count() << "->" << t->data << std::endl;
    t->data += 1;
    std::cout << t.use_count() << "->" << t->data << std::endl;
    return 1;
}

int share_ptr_test()
{
    std::shared_ptr<packages> t(new packages, [](packages* ptr)
        {
            delete ptr;
            ptr = nullptr;
            std::cout << "packages delete" << std::endl;
        });

    std::cout << t.use_count() << "->" << t->data << std::endl;
    t->data += 2;
    std::cout << t.use_count() << "->" << t->data << std::endl;
    t->data += 1;
    std::cout << t.use_count() << "->" << t->data << std::endl;
    share_ptr_test1(t);
    std::cout << t.use_count() << "->" << t->data << std::endl;
    return 1;
}


int main()
{
    a = new packages;
    a->data = 4;
    share_ptr_test();
    std::cout << a->data << std::endl;
    readerwriterqueue_test();
    return 0;
    std::vector<char*> v;
    v.push_back(NULL);
    int ach[20] = { 0 };
    for (int i = 0; i < 20; ++i)
    {
        ach[i] = i + 1;
    }
    char bch = ach[-1];

    datacenter::redisclient redisclient(clog, "172.30.70.23", 6379);
    redisclient.rpush("f_test", "value_test1");
    redisclient.rpush("f_test", "value_test2");
    /*  current_queue_test();
      char data[20];
      while (1)
      {
          std::cin>>data;
      }*/

    int a = rand();

#ifdef  __linux__

    long b = random();
    printf("%d -%d - %ld-%ld\n", a, a & 0xFFFF, b, b & 0xFFFF);

#endif //  __linux__

    return 0;
    logTes();
    std::atomic_uint64_t m_receive_count;
    std::cout << "count:" << m_receive_count << std::endl;
    return 0;
    zsetTest();
    int64_t date_time = datacenter::convert_to_timestamp(20200825091500000);


    std::cout << "[Version]" << datacenter::getVersion() << std::endl;
    std::cout << "[GitVersion]" << datacenter::getGitRevision() << std::endl;
    std::cout << "[ReleaseDateTime]" << datacenter::getBuildTime() << std::endl;
    //return redisTest();
    int64_t time = datacenter::get_time_ms();
    return 0;
}