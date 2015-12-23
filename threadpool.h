#ifndef CTHREADPOOL_H
#define CTHREADPOOL_H
#include "worker.h"
#endif

using namespace std;
class ThreadPool
{
public:

    ThreadPool(size_t threads = 4)
    {
        if (threads==0)
            threads=4;
        for (size_t i=0; i<threads; i++)
        {
            worker_ptr pWorker(new Worker);
            _workers.push_back(pWorker);
        }
    }
    ~ThreadPool() {}

    template<class _FN, class... _ARGS>
    void runAsync(_FN _fn, _ARGS... _args)
    {
        getFreeWorker()->appendFn(bind(_fn,_args...));
    }

    bool IsEmpty()
    {
        for (auto &it : _workers)
        {
            if (!it->isEmpty())
            {
                return false;
            }
        }
        return true;
    }

private:
    worker_ptr getFreeWorker()
    {
        worker_ptr pWorker;
        size_t minTasks = UINT32_MAX;
        for (auto &it : _workers)
        {
            if (it->isEmpty())
            {
                return it;
            }
            else if (minTasks > it->getTaskCount())
            {
                minTasks = it->getTaskCount();
                pWorker = it;
            }
        }
        return pWorker;
    }

    vector<worker_ptr> _workers;

};
