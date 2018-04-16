#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "Config.hpp"
#include "AddrArray.hpp"
#include "Task.hpp"
#include "HttpRequest.hpp"

#define CONTENTLENGTHMACRO "Content-Length:"
#define CONTENTTYPEMACRO   "Content-Type:"

void Task::Reset()
{
    m_state    = TASK_INITIAL;
    m_httpCode = 0;
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
        printf("Proxy: %d.%d.%d.%d %d %u %u\n", m_ip >> 24 , (m_ip >> 16) & 0xFF,
               (m_ip >> 8) & 0xFF, m_ip & 0xFF, m_port, m_retry, m_httpCode);

    } else if (m_state == TASK_AUTH) {
        printf("Auth: %d.%d.%d.%d %d %u %u\n", m_ip >> 24 , (m_ip >> 16) & 0xFF,
               (m_ip >> 8) & 0xFF, m_ip & 0xFF, m_port, m_retry, m_httpCode);

    } else if (m_state == TASK_FAKE) {
        printf("Fake: %d.%d.%d.%d %d %u %u\n", m_ip >> 24 , (m_ip >> 16) & 0xFF,
               (m_ip >> 8) & 0xFF, m_ip & 0xFF, m_port, m_retry, m_httpCode);
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
    char buf[4096];
    const char *url = request[m_retry % MAXREQUEST].Url;
    const char *webHost = request[m_retry % MAXREQUEST].Host;

    sprintf(buf, "GET http://%s/%s HTTP/1.0\r\nHost: %s\r\n"
                 "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                 "Pragma: no-cache\r\nExpires: 0\r\n\r\n", webHost, url, webHost);

    if (write(fd , buf, strlen(buf)) < 0) {
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

        End(fd);

        return;
    }

    m_length += len;
    m_buf[m_length] = 0;

    if (m_length < 12) {
        event_add(m_event, &timeout_seconds);
        return;
    }

    CheckHttp(fd);
}

void Task::GetHttpCode()
{
    if (m_buf[0] != 'H') return;
    if (m_buf[1] != 'T') return;
    if (m_buf[2] != 'T') return;
    if (m_buf[3] != 'P') return;
    if (m_buf[8] != ' ') return; 

    if ((m_buf[9] < '1') || (m_buf[9] > '5')) {
        return;
    }

    if ((m_buf[10] < '0') || (m_buf[10] > '9')) {
        return; 
    }

    if ((m_buf[11] < '0') || (m_buf[11] > '9')) {
        return; 
    }

    m_httpCode = ((m_buf[9] - '0') * 100) +
                 ((m_buf[10] - '0') * 10) +
                 ((m_buf[11] - '0'));
}

void Task::CheckHttp(int fd)
{
    char *p;

    if (m_state == TASK_SEND) {
        GetHttpCode();

        if (m_httpCode == 0) {

            m_state = TASK_ERROR;

            goto out;

        } else if (m_httpCode == 200) {

            m_state = TASK_CONTINUE;

        } else if (m_httpCode == 407) {

            m_state = TASK_AUTH;

            goto out;

        } else if ((m_httpCode >= 500) || (m_httpCode == 408)) {

            m_state = TASK_ABORT; // Next url

            goto out;

        } else {

            m_state = TASK_HTTP_ERR;

            goto out;

        }
    }

    if (m_state != TASK_CONTINUE) {
        printf("what: m_state = %d\n", m_state);
        exit(9);
    }

    p = strcasestr((char *) m_buf, CONTENTLENGTHMACRO);
    if (p != NULL) {
        p += sizeof(CONTENTLENGTHMACRO) - 1;

        if (*p == ' ') {
            p++;
        }

        if ((strstr(p, "\r") == NULL) && (strstr(p, "\n") == NULL)) {
            goto cont;
        }

        unsigned int len = 0;

        while (isdigit(*p)) {
            len *= 10;
            len += *p - '0';
            p++;
        }

        if (len != request[m_retry % MAXREQUEST].Length) {
            m_state = TASK_FAKE;
            goto out;
        }

        p = strcasestr((char *) m_buf, CONTENTTYPEMACRO);
        if (p != NULL) {
            p += sizeof(CONTENTTYPEMACRO) - 1;

            if (*p == ' ') {
                p++;
            }

            if ((strstr(p, "\r") == NULL) && (strstr(p, "\n") == NULL)) {
                goto cont;
            }

            if (strncasecmp(p, request[m_retry % MAXREQUEST].Type,
                            strlen(request[m_retry % MAXREQUEST].Type)) == 0) {

                m_state = TASK_SUCCESS;

            } else {

                m_state = TASK_FAKE;

            }

            goto out;
        }
    }

cont:

    if (m_length >= MAXHEADERLENGTH) {

        m_state = TASK_FAKE;

    } else if (strstr((char *) m_buf, "\r\n\r\n")) {

        m_state = TASK_FAKE;

    } else if (strstr((char *) m_buf, "\n\n")) {

        m_state = TASK_FAKE;

    } else { // TASK_CONTINUE

        event_add(m_event, &timeout_seconds);

        return;

    }

out:

    End(fd);
   
    return;
}
