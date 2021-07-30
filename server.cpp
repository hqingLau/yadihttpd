#include "server.h"
#include "commonfunc.h"
#include "log.h"
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


bool yadi::Server::run()
{
    servSockfd = socket(AF_INET,SOCK_STREAM,0);
    if(servSockfd == -1)
    {
        sprintf(logBuffer,"%s: %s","socket",strerror(errno)); 
        YADILOGERROR(logBuffer); 
        handle_error("socket");
    }
        
    sockaddr_in sa;
    bzero(&sa,sizeof(sockaddr));
    sa.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&sa.sin_addr.s_addr);
    //sa.sin_addr.s_addr = inet_addr(ip); //INADDR_ANY;
    sa.sin_port = htons(port);
    if(bind(servSockfd,(sockaddr *)&sa,sizeof(sa))==-1)
    {
        sprintf(logBuffer,"%s: %s","bind",strerror(errno)); 
        YADILOGERROR(logBuffer); 
        handle_error("bind");
    }
        
    if(listen(servSockfd,10)==-1) 
    {
        sprintf(logBuffer,"%s: %s","listen",strerror(errno)); 
        YADILOGERROR(logBuffer); 
        handle_error("listen");
    }

    sprintf(logBuffer,"%s:%d waiting for request...",ip,port);
    puts(logBuffer);
    YADILOGINFO(logBuffer);
    sockaddr_in sacli;
    socklen_t saclilen = sizeof(sacli);
    int cfd;
    while(1)
    {
        cfd = accept(servSockfd,(sockaddr *)&sacli,&saclilen);
        if(cfd == -1) 
        {
            sprintf(logBuffer,"%s: %s","accept",strerror(errno)); 
            YADILOGERROR(logBuffer); 
            handle_error("accept");
        }
        inet_ntop(AF_INET,&sacli.sin_addr,cliip,sizeof(cliip));
        cliport = ntohs(sacli.sin_port);
        sprintf(logBuffer,"%s:%d connected",cliip,cliport); 
        YADILOGINFO(logBuffer); 
        char req_content[1024];
        int reqlen = recv(cfd,req_content,1023,0);
        req_content[reqlen] = 0;
        printf("%s\n",req_content);
        printf("recv done!\n");
        shutdown(cfd,SHUT_RDWR);
    }
    return true;
}