#ifndef __TASKLIST_HPP__
#define __TASKLIST_HPP__    1 

#include <list>

#include "Config.hpp"
#include "AddrArray.hpp"

#if defined(SOCKS4)
#include "Socks4Task.hpp"
#elif defined(SOCKS5)
#include "Socks5Task.hpp"
#else
#include "Task.hpp"
#endif

using namespace std;

class Task;

class TaskList {
public:
    unsigned int mIndex;
    unsigned int mArrayIndex;
    unsigned int mPortIndex;
    unsigned int mTaskNum;
    AddrArray   *mAddrArray;
    Task        *mTask;
    list<Task *> mRetryList;

public:

    TaskList(AddrArray *addrArray);

    Task *GetNewTask();

private:

    void GetNextTask(Task *task);

    unsigned int GetNextIndex();

    bool ValidateIndex();
};

#endif // #ifndef __TASKLIST_HPP__
