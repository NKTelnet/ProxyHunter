#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netdb.h>
#include <event2/event.h>

#include "AddrArray.hpp"
#include "TaskList.hpp"
#include "Config.hpp"

struct timeval timeout_seconds = {TIMEOUTSECONDS, 0};

void setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
        printf("fcntl: %d\n", errno);
        exit(3);
    }
}

AddrArray addrArray;

int get_new_connection(Task *task)
{
    int newfd = socket(AF_INET, SOCK_STREAM, 0);
    if (newfd < 0) {
        printf("can not open socket: %d\n", errno);
        exit(4);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_addr.s_addr = htonl(task->m_ip);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(task->m_port);

    setnonblocking(newfd);

    if (connect(newfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        if (errno != EINPROGRESS)
        {
            printf("can not connect: %d\n", errno);
            close(newfd);
            return -1;
        }
    }

    return newfd;
}

void cb_func(int fd, short what, void *arg)
{
    Task *task = (Task *) arg;

    if (what & EV_TIMEOUT) {
        if (task->m_engineTask) {

            task->EngineTaskHandle();
 
        } else {

            task->HandleTimeout(fd);

        }

        return;

    } else if (what & EV_WRITE) {

        task->HandleWrite(fd);

        return; 

    } else if (what & EV_READ) {

        task->HandleRead(fd);

        return;

    } else {
        printf("what = %d\n", what);
        exit(5);
    }
}

int add_new_event(Task *engineTask)
{
    int fd;
    Task *task;

    while (1) {
        task = engineTask->m_taskList->GetNewTask(); 
        if (task == NULL) {
            return -1;
        }

        fd = get_new_connection(task);
        if (fd < 0) {
            delete task;
            continue;
        } else {
            break;
        }
    }

    struct event_base *base = event_get_base(engineTask->m_event);
    struct event *ev = event_new(base, fd, EV_WRITE, cb_func, task);
    task->m_event = ev;
    event_add(ev, &timeout_seconds);

    return 0;
}

void *work_thread(void *arg)
{
    TaskList taskList(&addrArray);

    struct event_base *base = event_base_new();

    Task *task = new Task(&taskList, 1); //engine Task
    if (task == NULL) {
        printf("can not new engine task\n");
        exit(6); 
    }

    struct event *ev = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, cb_func, task);
    task->m_event = ev;

    struct timeval engine_timeout = {0, (1000000 / CONNECTION_HZ)};

    event_add(ev, &engine_timeout);

    event_base_dispatch(base);

    return NULL;
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);

    fprintf(stderr, "MAXRETRY = %u\n", MAXRETRY);

    fprintf(stderr, "HZ = %u\nTHREADS_NUM = %u\n", CONNECTION_HZ, THREADS_NUM);

    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = (1 * 1024 * 1024);
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
        printf("setrlimit: %d\n", errno);
        //exit(1);
    }

    int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    fprintf(stderr, "_SC_NPROCESSORS_ONLN = %d\n", cpu_num);
    if (cpu_num < 1) cpu_num = 1;

    cpu_num = THREADS_NUM;

    pthread_t work_thread_tid[cpu_num];
 
    for (int i = 0; i < cpu_num; i++) {
        if (pthread_create(&work_thread_tid[i], NULL, work_thread, NULL) != 0) {
            printf("can not create work thread\n");
            exit(2);
        }
    }

    for (int i = 0; i < cpu_num; i++) {
        pthread_join(work_thread_tid[i], NULL);
    }
}
