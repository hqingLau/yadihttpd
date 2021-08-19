#include "server.h"
#include "log.h"
#include <string.h>
#include <signal.h>

void handle_pipe(int sig){}

int main(int argc,char *argv[])
{
    if(argc<4)
    {
        printf("usage: %s <port> <websit root> <log dir>\n要实现创建好文件夹和响应静态文件\n如：./yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog\n",argv[0]);
        exit(1);
    }
    daemon(1,0);
    struct sigaction action;
    action.sa_handler = handle_pipe;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE,&action,NULL);
    int port = atoi(argv[1]);
    char *webroot = (char *)malloc(128);
    char *logdir = (char *)malloc(128);
    strcpy(webroot,argv[2]);
    strcpy(logdir,argv[3]);
    if(port==0)
    {
        printf("Port need a number. Got %s\n",argv[1]);
        exit(1);
    }
    YADILOGINFO("start main function");
    yadi::Server server("localhost",port,webroot,logdir);
    server.run();
    free(webroot);
    free(logdir);
    return 0;
}
