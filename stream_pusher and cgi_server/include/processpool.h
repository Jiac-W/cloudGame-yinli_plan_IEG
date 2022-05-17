#ifndef PROCESSPOLL_H
#define PROCESSPOLL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <chrono>
#include <iostream>
#include <sys/time.h>

// 描述一个子进程的类
class process
{
    public:
        process() : m_pid(-1){ }
    public:
        pid_t   m_pid;      
        int m_pipefd[2];    // 父进程与子进程之间通信所用管道
};

// 进程池类，其模板参数是处理逻辑任务的类。
template< typename T >
class processpool
{
    private:
        // 将构造函数定义为私有的，因此只能通过后面的create 静态函数来创建processpool 实例
        processpool( int listenfd, int process_number = 8 );
    public:
        // 单体模式，以保证程序最多创建一个processpool 实例，这是程序正确处理信号的必要条件
        static processpool< T >* create( int listenfd, int process_number = 8 )
        {
            if( !m_instance )
            {
                m_instance = new processpool< T >(listenfd, process_number );
            }
            return m_instance;
        }

        ~processpool()
        {
            delete [] m_sub_process;
        }

        // 启动进程池
        void run();

    private:
        void setup_sig_pipe();
        void run_parent();
        void run_child();

    private:
        static const int MAX_PROCESS_NUMBER = 16;        // 进程池允许的最大子进程数量
        static const int USER_PER_PROCESS = 65536;       // 每个子进程最多能处理的客户数量
        static const int MAX_EVENT_NUMBER = 10000;       // epoll最大能处理的事件数

        int m_process_number;       // 进程池中进程总数
        int m_idx;                  // 子进程在池中的序号，从0开始
        int m_epollfd;              // 每个进程都有一个epoll 内核事件表，用m_epollfd 标识
        int m_listenfd;             // 监听socket
        int m_stop;                 // 子进程通过m_stop 来决定是否停止运行
        process* m_sub_process;     // 保存所有子进程的描述信息
        static processpool< T >* m_instance;    // 进程池静态实例
};

template< typename T >
processpool< T >* processpool< T >::m_instance = NULL; // 静态成员，类内声明，类外初始化

static int sig_pipefd[2];                              // 用于处理信号的管道，以实现统一事件源。后面称之为信号管道

static int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

static void addfd( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

// 从epollfd 标识的epoll 内核事件表中，删除fd 上的所有注册事件
static void removefd( int epollfd, int fd )
{
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0);
    close( fd );
}

// 信号处理函数：被触发的时候，往信号管道入口端sig_pipefd[1]处，灌入数据信号
static void sig_handler( int sig )
{
    int save_errno = errno;
    int msg = sig;
    send( sig_pipefd[1], (char*) &msg, 1, 0);       
    errno = save_errno;
}

static void addsig( int sig, void(handler)(int), bool restart = true )
{
    struct sigaction sa;
    memset( &sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if(restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1);
}

// 构造进程池。参数listenfd 是监听socket，它必须在创建进程池之前被创建，否则子进程无法直接引用它。
template< typename T >
processpool< T >::processpool( int listenfd, int process_number )
    : m_listenfd( listenfd ), m_process_number( process_number ), m_idx( -1 ), m_stop( false )
{
    assert( (process_number > 0) && (process_number <= MAX_PROCESS_NUMBER));

    m_sub_process = new process[ process_number ];

    assert( m_sub_process );

    // 创建process_number个子进程，并分别建立它们和父进程之间的管道
    for( int i=0; i<process_number; ++i)
    {
        int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd ); // 建立双向的套接字对通道
        assert ( ret == 0);

        m_sub_process[i].m_pid = fork();            // fork()调用，在父进程中返回子进程的PID，在子进程中则返回 0
        assert( m_sub_process[i].m_pid >= 0);

        if(m_sub_process[i].m_pid > 0)     
        {
            close( m_sub_process[i].m_pipefd[1]);
            continue;
        }
        else                                
        {
            close( m_sub_process[i].m_pipefd[0]);
            m_idx = i;
            break;
        }
    }
}

// 统一事件源
template< typename T >
void processpool< T >::setup_sig_pipe()
{
    m_epollfd = epoll_create(5);        // 创建epoll 内核事件监听表
    assert( m_epollfd != -1);

    int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, sig_pipefd );
    assert( ret != -1);

    setnonblocking( sig_pipefd[1]);     // 将信号管道 入口端fd 设置为非堵塞
    addfd( m_epollfd, sig_pipefd[0] );  // 将信号管道 出口端fd 加入内核事件表

    // 设置绑定信号处理函数
    addsig( SIGCHLD, sig_handler );
    addsig( SIGTERM, sig_handler );
    addsig( SIGINT, sig_handler );
    addsig( SIGPIPE, SIG_IGN );
}

// 根据该值判断接下来要运行的是父进程代码，还是子进程代码，父进程中：m_idx 值被标记为 -1
template< typename T >
void processpool< T >::run()
{
    if(m_idx != -1)
    {
        run_child();
        return;
    }
    run_parent();
}

// 子进程执行的内容
template< typename T >
void processpool< T >::run_child()
{   
    setup_sig_pipe();   // 统一事件源，并在子函数内部初始化对应的内核事件表

    // 每个子进程都通过其在进程池中的序号m_idx 找到与父进程通信的套接字m_pipefd[0] 或 m_pipefd[1]
    int pipefd = m_sub_process[m_idx].m_pipefd[1];

    // 子进程需要监听管道文件描述符pipefd，因为父进程将通过它来通知子进程accept 新连接
    addfd( m_epollfd, pipefd );

    epoll_event events[ MAX_EVENT_NUMBER ];
    T* users = new T[ USER_PER_PROCESS ];

    assert( users );
    int number = 0;
    int ret = -1;

    while( ! m_stop )
    {
        number = epoll_wait( m_epollfd, events, MAX_EVENT_NUMBER, -1);  // 返回有事件发生的socket个数。
        if((number < 0) && (errno != EINTR))
        {
            printf( "epoll failure\n");
            break;
        }

        for(int i=0; i<number; i++)
        {
            int sockfd = events[i].data.fd;
            // 对到来的事件判断，这里有三种，一是数据，二是新连接，三是信号。
            // 1.新连接
            if((sockfd == pipefd ) && (events[i].events & EPOLLIN))
            {
                int client = 0;
                ret = recv( sockfd, (char*)&client, sizeof(client), 0);
                if( ((ret < 0) && (errno != EAGAIN )) || ret == 0)
                { continue; }
                else
                {
                    struct sockaddr_in client_address;
                    socklen_t client_addrlength = sizeof( client_address );
                    int connfd = accept( m_listenfd, (struct sockaddr*)&client_address, &client_addrlength );   // 子进程内接受新连接
                    if(connfd < 0)
                    {
                        printf( "errno is: %d\n", errno );
                        continue;
                    }

                    addfd( m_epollfd, connfd );     // 子进程中将新接受的fd 加到该子进程维护的内核事件表中。

                    // 模板类T必须实现init 方法，以初始化一个客户连接。
                    // 直接使用connfd 来索引逻辑处理对象（T类型的对象），以提高程序效率。
                    users[connfd].init( m_epollfd, connfd, client_address );
                }
            }
            // 2.如果是信号管道
            else if( ( sockfd == sig_pipefd[0] ) && (events[i].events & EPOLLIN ))
            {
                int sig;
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof(signals), 0);

                if(ret <= 0)
                {
                    continue;
                }
                else
                {
                    for( int i=0; i<ret; ++i)
                    {
                        switch( signals[i])
                        {
                            case SIGCHLD: // 子进程终止时会向父进程发送SIGCHLD信号，告知父进程回收自己，
                            {             // 但该信号的默认处理动作为忽略，因此父进程仍然不会去回收子进程，需要捕捉处理实现子进程的回收。
                                pid_t pid;
                                int stat;
                                while( (pid = waitpid(-1, &stat, WNOHANG )) > 0)
                                {
                                    continue;
                                }
                                break;
                            }

                            case SIGTERM:
                            case SIGINT:
                            {
                                m_stop = true;
                                break;
                            }
                            default:{ break; }
                        }
                    }
                }
            }
            // 3.数据到来的情况
            else if( events[i].events & EPOLLIN )
            {
                users[sockfd].process();
            }
            else{ continue; }
        }
    }

    delete [] users;    
    users = NULL;
    close( pipefd );
    // close( m_listenfd ); // 注意：应该由m_listenfd 的创建者，来关闭这个文件描述符。
    close( m_epollfd );     // 原则：所谓的“对象”，例如一个文件描述符、或者一段堆内存，由哪个函数创建，就应该由那个函数销毁
}

// 父进程执行的内容
template< typename T >
void processpool< T >::run_parent()
{
    setup_sig_pipe();
    addfd(m_epollfd, m_listenfd );

    epoll_event events[ MAX_EVENT_NUMBER ];
    int sub_process_counter = 0;
    int new_conn = 1;
    int number = 0;
    int ret = -1;

    while( !m_stop )
    {
        number = epoll_wait( m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if( (number < 0) && (errno != EINTR))
        {
            printf( "epoll failure\n");
            break;
        }

        for( int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            // 1.如果有新连接到来，
            if( sockfd == m_listenfd)
            {
                // 就采用Round Robin方式将其分配给一个子进程处理
                int i = sub_process_counter;
                do
                {
                    if(m_sub_process[i].m_pid != -1)    // 如果是子进程
                    {
                        break;
                    }
                    i = (i +1) % m_process_number;
                }
                while (i != sub_process_counter);

                if(m_sub_process[i].m_pid == -1)        // 如果是父进程
                {
                    m_stop = true;
                    break;
                }
                sub_process_counter = (i+1) % m_process_number;
                send( m_sub_process[i].m_pipefd[0], ( char* )&new_conn, sizeof( new_conn ), 0); // 往第[i]个子进程m_sub_process[i]对应的管道，灌入一个数据代表信号触发。
            }
            // 2.处理父进程接收到的信号
            else if( (sockfd == sig_pipefd[0] ) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof( signals ), 0);

                if(ret <= 0)
                {
                    continue;
                }
                else
                {
                    for(int i=0; i< ret; ++i)
                    {
                        switch( signals[i])
                        {
                            case SIGCHLD:
                            {
                                pid_t pid;
                                int stat;
                                while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                                {
                                    for(int i=0; i<m_process_number; ++i)
                                    {
                                        // 如果进程池中第i个子进程退出了，则主进程关闭相应的通信管道，并设置相应的m_pid为-1,以标记该子进程已经退出
                                        if( m_sub_process[i].m_pid == pid)
                                        {
                                            printf( "child %d join\n", i);
                                            close(m_sub_process[i].m_pipefd[0]);
                                            m_sub_process[i].m_pid = -1;
                                        }
                                    }
                                }

                                // 如果所有子进程都已经退出了，则父进程也退出
                                m_stop = true;
                                for( int i=0; i<m_process_number; ++i)
                                {
                                    if(m_sub_process[i].m_pid != -1)    //但凡还有一个子进程m_pid没有被置为-1,即代表还没退出
                                    {
                                        m_stop = false;
                                    }
                                }
                                break;
                            }

                            case SIGTERM:
                            case SIGINT:
                            {
                                // 如果父进程接收到终止信号，那么就杀死所有子进程，并等待它们全部结束。
                                // TODO：通知子进程结束的更好办法是：向父、子进程之间的通信管道发送特殊数据，有待实现
                                printf( "kill all the child now\n");
                                for( int i=0; i<m_process_number; i++)
                                {
                                    int pid = m_sub_process[i].m_pid;
                                    if(pid != -1)
                                    {
                                        kill(pid, SIGTERM);
                                    }
                                }
                                break;
                            }
                            default:{ break; }
                        }
                    }
                }
            }
            else{ continue; }
        }
    }

    // close(m_listenfd);   //注意：应该由m_listenfd 的创建者，来关闭这个文件描述符。
    close(m_epollfd);
}

#endif  //PROCESSPOLL_H