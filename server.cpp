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
        
        char head[128];
        int ditmpi = 0;
        while(!(req_content[ditmpi]=='\r'&&req_content[ditmpi+1]=='\n'))
        {
            ditmpi++;
        }
        strncpy(head,req_content,ditmpi);
        head[ditmpi] = 0;
        ditmpi = 0;
        while(isspace(head[ditmpi])) ++ditmpi;
        int ditmpj = ditmpi;
        while(!isspace(head[ditmpi])) ++ditmpi;
        strncpy(method,&head[ditmpj],ditmpi-ditmpj);
        method[ditmpi-ditmpj] = 0;
        sprintf(logBuffer,"req method: %s",method); 
        YADILOGINFO(logBuffer); 
        if(strncmp(method,"GET",3)!=0)
        {

            sprintf(logBuffer,"only GET method supported now! Got: %s\n",method);
            YADILOGINFO(logBuffer); 
            send(cfd,logBuffer,strlen(logBuffer),0);
            shutdown(cfd,SHUT_RDWR);
            continue;
        }
        while(isspace(head[ditmpi])) ++ditmpi;
        ditmpj =ditmpi;
        while(!isspace(head[ditmpi])) ++ditmpi;
        strncpy(filepath,&head[ditmpj],ditmpi-ditmpj);
        filepath[ditmpi-ditmpj] = 0;
        sprintf(logBuffer,"req filepath: %s",filepath); 
        YADILOGINFO(logBuffer);
        sprintf(absoluteFilePath,"%s%s",rootdir,filepath);
        unsigned char *fileBuffer = (unsigned char *)malloc(sizeof(unsigned char)*1024*100);
        FILE *direqfd = fopen(absoluteFilePath,"rb");
        if(direqfd==NULL)
        {
            sprintf(logBuffer,"404 no such file."); 
            YADILOGINFO(logBuffer);
            send(cfd,logBuffer,strlen(logBuffer),0);
            continue;
        }
        size_t fileBufferlen = fread(fileBuffer,1,1024*100,direqfd);
        printf("file buffer len: %d\n",fileBufferlen);
        char outputhead[1024];
        char suffix[16];
        char * pos = strrchr(filepath,'.');
        snprintf(suffix,15,"%s",&filepath[pos-filepath+1]);
        printf("req type: %s\n",suffix);
        if(strncmp(suffix,"jpg",3)==0)
            sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:image/jpeg\r\n\r\n");
        else if(strncmp(suffix,"html",4)==0)
            sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
        else
            sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:text/plain\r\n\r\n");
        send(cfd,outputhead,strlen(outputhead),0);
        send(cfd,fileBuffer,fileBufferlen,0);
        
        free(fileBuffer);
        fclose(direqfd);
        shutdown(cfd,SHUT_RDWR);
    }
    return true;
}