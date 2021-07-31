#include "server.h"
#include "log.h"

int main(int argc,char *argv[])
{
    if(argc<2)
    {
        printf("usage: %s <port>\n",argv[0]);
        exit(1);
    }
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