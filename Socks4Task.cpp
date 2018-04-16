#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "Config.hpp"
#include "AddrArray.hpp"
#include "Socks4Task.hpp"

void Task::Reset()
{
    m_state    = TASK_INITIAL;
    m_length   = 0;
    m_buf[0]   = 0;
}

Task::Task(TaskList *taskList)
{
    m_taskList   = taskList;
    m_port       = PORTS[taskList->mPortIndex];
    m_event      = NULL;
    m_retry      = 0;
    m_engineTask = 0;

    Reset();
}

Task::Task(Task &task)
{
    m_ip         = task.m_ip;
    m_port       = task.m_port;
    m_event      = NULL;
    m_taskList   = task.m_taskList;
    m_retry      = 0;
    m_engineTask = 0;

    Reset();
}

Task::Task(TaskList *taskList, unsigned int unused)
{
    m_engineTask = 1;
    m_taskList = taskList;
}

Task::~Task()
{
    if (m_engineTask == 0) {
        m_taskList->mTaskNum--;
    }
}

void Task::Print()
{
    if (m_state == TASK_SUCCESS) {
        printf("Proxy: %d.%d.%d.%d %d %u\n", m_ip >> 24 , (m_ip >> 16) & 0xFF,
               (m_ip >> 8) & 0xFF, m_ip & 0xFF, m_port, m_retry);

    } else if (m_state == TASK_AUTH) {
        printf("Auth: %d.%d.%d.%d %d %u\n", m_ip >> 24 , (m_ip >> 16) & 0xFF,
               (m_ip >> 8) & 0xFF, m_ip & 0xFF, m_port, m_retry);

    } else if (m_state == TASK_FAKE) {
        printf("Fake: %d.%d.%d.%d %d %u\n", m_ip >> 24 , (m_ip >> 16) & 0xFF,
               (m_ip >> 8) & 0xFF, m_ip & 0xFF, m_port, m_retry);

    }
}

void Task::EngineTaskHandle()
{
    if ((add_new_event(this) < 0) &&
        (m_taskList->mTaskNum == 0)) {
        event_del(m_event);
        event_free(m_event);
        m_event = NULL;
        return;
    }

    return;
}

void Task::End(int fd)
{
    close(fd);

    event_del(m_event);
    event_free(m_event);
    m_event = NULL;

    if ((m_state == TASK_SEND) ||
        (m_state == TASK_CONTINUE) ||
        (m_state == TASK_ABORT)) {

        if (m_retry < MAXRETRY) {
            m_retry++;

            Reset();

            m_taskList->mRetryList.push_back(this);

            return;
        }
    }

    Print();

    delete this;

    return;
}

void Task::HandleTimeout(int fd)
{
    End(fd);
}

void Task::HandleWrite(int fd)
{
    unsigned char request[64] = {4, 1, 0, 80};

    /* amazon.com ip: 54.239.25.192*/
    request[4] = 54;
    request[5] = 239;
    request[6] = 25;
    request[7] = 192;

    request[8] = 'c'; 
    request[9] = 'r'; 
    request[10] = 'y'; 
    request[11] = 's'; 
    request[12] = 't'; 
    request[13] = 'a'; 
    request[14] = 'l';

    request[15] = 0; 

    if (write(fd , request, 16) < 0) {
        if (errno != ECONNREFUSED) {
            printf("write error: %d\n", errno);
        }

        End(fd);
        return;
    }

    m_state = TASK_SEND;

    struct event_base *base = event_get_base(m_event);

    event_del(m_event);
    event_free(m_event);
    m_event = NULL;

    struct event *ev = event_new(base, fd, EV_READ, cb_func, this);
    m_event = ev;
    event_add(ev, &timeout_seconds);

    return;
}

void Task::HandleRead(int fd)
{
    int len = read(fd, m_buf + m_length, MAXHEADERLENGTH - m_length);

    if (len <= 0) {
        m_state = TASK_ABORT;

        goto out;
    }

    m_length += len;
    m_buf[m_length] = 0;

    if ((m_buf[0] != 0) && (m_buf[0] != 4)) {
        m_state = TASK_ERROR;

        goto out;
    }

    if (m_length < 8) {
        event_add(m_event, &timeout_seconds);
        return;
    }

    if (m_length > 8) {
        m_state = TASK_ERROR;

        goto out;
    }

    if ((m_buf[1] == 90) || (m_buf[1] == 91) || (m_buf[1] == 92)) {

        m_state = TASK_SUCCESS;

    } else if (m_buf[1] == 93) {

        m_state = TASK_AUTH;

    } else {

        m_state = TASK_FAKE;

    }

out:

    End(fd);

    return;
}
