#ifndef __SOCKS4TASK_HPP__
#define __SOCKS4TASK_HPP__    1

#include <sys/time.h>
#include <event2/event.h> 

#include "Config.hpp"
#include "TaskList.hpp"

#define MAXHEADERLENGTH 128

class TaskList;
class Task;

int add_new_event(Task *engineTask);
void cb_func(int fd, short what, void *arg);

extern struct timeval timeout_seconds;

enum TaskState {
    TASK_INITIAL  = 0,
    TASK_SEND     = 1,
    TASK_ERROR    = 2,
    TASK_CONTINUE = 3,
    TASK_SUCCESS  = 4,
    TASK_FAKE     = 5,
    TASK_ABORT    = 6,
    TASK_AUTH     = 7,
};

class Task {
public:
    unsigned int   m_ip;
    unsigned short m_port;
    unsigned short m_protocol;
    struct event  *m_event;
    TaskList      *m_taskList;
    unsigned int   m_engineTask;
    unsigned int   m_state;
    unsigned int   m_retry;
    int            m_length;
    unsigned char  m_buf[MAXHEADERLENGTH + 1];

public:
    Task(TaskList *taskList); 

    Task(Task &task);

    Task(TaskList *taskList, unsigned int unused);

    void Reset();

    void HandleRead(int fd);

    void HandleWrite(int fd);

    void HandleTimeout(int fd);

    void EngineTaskHandle();

    void Print();

    void End(int fd);

    ~Task();
};

#endif // #ifndef __SOCKS4TASK_HPP__
