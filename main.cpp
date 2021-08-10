#include "server.h"
#include "log.h"
#include <signal.h>

void handle_pipe(int sig){}

int main(int argc,char *argv[])
{
    if(argc<2)
    {
        printf("usage: %s <port>\n",argv[0]);
        exit(1);
    }
    daemon(1,0);
    struct sigaction action;
    action.sa_handler = handle_pipe;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE,&action,NULL);
    int port = atoi(argv[1]);
    if(port==0)
    {
        printf("Port need a number. Got %s\n",argv[1]);
        exit(1);
    }
    YADILOGINFO("start main function");
    yadi::Server server("localhost",port);
    server.run();
    
    return 0;
}
