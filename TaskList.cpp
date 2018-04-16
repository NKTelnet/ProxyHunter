#include <unistd.h>
#include <stdlib.h>

#include "Config.hpp"
#include "AddrArray.hpp"
#include "TaskList.hpp"

TaskList::TaskList(AddrArray *addrArray)
{
    mIndex = MINADDRINDEX; 
    mPortIndex = 0;

    mTaskNum = 0;

    mAddrArray  = addrArray;
    mArrayIndex = addrArray->GetNextArrayIndex();

    mTask = new Task(this);

    if (mTask == NULL) {
        printf("no mem\n");
        exit(7);
    }

    mTask->m_ip = GetNextIndex();
}

Task *TaskList::GetNewTask()
{
    if (!mRetryList.empty()) {
        Task *task = mRetryList.front();
        mRetryList.pop_front();
        return task;
    }

    Task *task = new Task(*mTask);

    if (task == NULL) {
        printf("no mem\n");
        exit(8);
    }

    mTaskNum++;

    if (task->m_ip == 0) {
        delete task;
        task = NULL;
    } else {
        GetNextTask(mTask);
    }

    return task;
}

void TaskList::GetNextTask(Task *task)
{
    task->m_ip = GetNextIndex();
    if (task->m_ip == 0) {
        mPortIndex++;
        if (mPortIndex >= PORTS_NUM) {
            mPortIndex = 0;

            mArrayIndex = mAddrArray->GetNextArrayIndex(); 
        }

        task->m_port = PORTS[mPortIndex];

        mIndex = MINADDRINDEX;

        task->m_ip = GetNextIndex();

    }
}

unsigned int TaskList::GetNextIndex()
{
    if (mArrayIndex == 0) {
        return 0;
    }

    while (1) {
        if (mIndex >= MAXADDRINDEX) {
            return 0;
        }

        if (ValidateIndex()) {
            unsigned int ret = (mArrayIndex << 24) + (mIndex);

            mIndex++;

            return ret;
        }

        mIndex++;
    }
}

bool TaskList::ValidateIndex()
{
    if (mArrayIndex == 192) {
        if ((mIndex >> 16) == 168) {
            mIndex = (169 << 16);
            mIndex--;
            return false; 
        }
    } else if (mArrayIndex == 172) {
        if (((mIndex >> 16) >= 16) &&
            ((mIndex >> 16) < 32)) {
            mIndex = (32 << 16);
            mIndex--;
            return false;
        }
    }

    return true;
}
