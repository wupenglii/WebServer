#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "./threadpool/threadpool.h"
#include "./http/http_conn.h"
#include "./CGImysql/sql_connection_pool.h"
#include "./lock/locker.h"
#include "./timer/timer.h"

#define MAX_FD 65536               /*最大文件描述符*/
#define MAX_EVENT_NUMBER 10000     //最大事件数
#define TIMESLOT   5            //最小超时单位

#define SYNSQL       //同步数据库校验
//#define CGISQLPOOL //CGI数据库校验
//#define ET        //边缘触发非阻塞
#define LT  //水平触发阻塞

//三个函数在http_conn.cpp中定义，改变链接属性
extern int addfd(int epollfd,int fd, bool one_shot);
extern int remove(int epollfd,int fd);
extern int setnonblocking(int fd);

//设置定时器相关参数
static int pipefd[2];
static sort_timer_lst timer_lst;
static int epollfd = 0;

//设置信号函数
// void addsig(int sig, void (handler)(int),bool restart = true){
//     struct sigaction sa;
//     memset( &sa, '\0',sizeof( sa ));
//     sa.sa_handler = handler;
//     if( restart ){
//         sa.sa_flags |= SA_RESTART;
//     }
//     sigfillset( &sa.sa_mask);
//     assert(sigaction(sig,&sa,NULL)!=-1);
// }

//定时处理任务，重新定时以不断触发SIGALRM信号
void timer_headler(){
    timer_lst.tick();
    alarm(TIMESLOT);
}

//定时器回调函数，删除非活动连接在socket上的注册事件,并关闭
void cb_func(client_data *user_data){
    epoll_ctl(epollfd,EPOLL_CTL_DEL,user_data->sockfd,0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
    printf("close fd %d",user_data->sockfd);
    //Log::get_instance()->flush();
}

void show_error(int connfd, const char* info){
    printf("%s",info);
    send(connfd,info,strlen(info),0);
    close(connfd);
}

int main(int argc,char * argv[]){

    if(argc <= 1){
        printf("usage: %s ip_address port_number\n",basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);

    /*忽略SIGPIPE信号*/
    //addsig( SIGPIPE, SIG_IGN);

    //创建数据库连接池
    connection_pool *connPool = connection_pool::GetInstance();
    connPool->init("localhost","root","123","dataDb",3306,8);

    /*创建线程池*/
    threadpool< http_conn >* pool = NULL;
    try{
        pool = new threadpool< http_conn >(connPool);
    }catch(...){
        return 1;
    }

    /*预先为每个可能的客户连接分配一个http_conn对象*/
    http_conn* users = new http_conn[MAX_FD];
    assert(users);
    int user_count = 0;

#ifdef SYNSQL
    //初始化数据库读取表
    users->initmysql_result(connPool);
#endif

#ifdef CGISQLPOOL
    //初始化数据库读取表
    users->initresultFile(connPool);
#endif

    //创建套接子,返回listenfd
    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd >= 0);
    struct linger tmp = { 1,0 };
    setsockopt( listenfd,SOL_SOCKET,SO_LINGER,&tmp,sizeof(tmp));


    int ret = 0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    //设置端口复用，绑定端口
    int flag = 1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    ret = bind(listenfd, (struct sockaddr*)&address,sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);

    //创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd,listenfd, false);
    http_conn::m_epollfd = epollfd;

    while(1){
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER,-1);
        if( (number < 0) &&(errno != EINTR)){
            printf("epoll failure");
            break;
        }

        for(int i = 0; i < number; i++){
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);

                int connfd = accept(listenfd, (struct sockaddr *)&client_address,&client_addrlength);
                if( connfd < 0 ){
                    printf("errno is:%d\n",errno);
                    continue;
                }
                if( http_conn::m_user_count >= MAX_FD ){
                    show_error( connfd, "Internal server busy");
                }

                /* 初始化客户连接*/
                users[connfd].init( connfd, client_address );

            }else if(events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                /*如果有异常，直接关闭客户连接*/
                users[sockfd].close_conn();
            }
            //处理客户连接上接收到的数据
            else if(events[i].events & EPOLLIN){
                /*根据读的结果，决定是将任务添加到线程池，还是关闭连接*/
                if( users[sockfd].read()  ){
                    pool->append( users + sockfd);
                }else{
                    users[sockfd].close_conn();
                }

            }
            else if(events[i].events & EPOLLOUT){
                /*根据写的结果，决定是否关闭连接*/
                if( !users[sockfd].write()){
                    users[sockfd].close_conn();
                }else{

                }
            }

        }
        
    }
    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;
    return 0;
}