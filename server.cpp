#include "server.h"
#include "commonfunc.h"
#include "log.h"
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
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
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = servSockfd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,servSockfd,&ev);

    int readyFdNum;
    int curfd;
    for(;;)
    {
        readyFdNum = epoll_wait(epollfd,srvEvents,epollEvNum,-1);
        for(int i=0;i<readyFdNum;i++)
        {
            curfd = srvEvents[i].data.fd;
            if(curfd==servSockfd)
            {
                sockaddr_in sacli;
                socklen_t saclilen = sizeof(sacli);
                int cfd;
                cfd = accept(servSockfd,(sockaddr *)&sacli,&saclilen);
                if(cfd == -1) 
                {
                    if(errno==EAGAIN||errno==EWOULDBLOCK) continue;
                    sprintf(logBuffer,"%s: %s","accept",strerror(errno)); 
                    YADILOGERROR(logBuffer); 
                    continue;
                }
                ClientInfo *cliinfo = (ClientInfo *)malloc(sizeof(ClientInfo));
                inet_ntop(AF_INET,&sacli.sin_addr,cliinfo->cliip,sizeof(cliinfo->cliip));
                cliinfo->cliport = ntohs(sacli.sin_port);
                cliinfo->cfd = cfd;
                sprintf(logBuffer,"%s:%d connected",cliinfo->cliip,cliinfo->cliport); 
                YADILOGINFO(logBuffer); 
                printf("%s\n",logBuffer);
                epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = cfd;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,cfd,&ev);
                int tfd = timerfd_create(CLOCK_MONOTONIC,0);
                itimerspec time_intv;
                time_intv.it_value.tv_sec = 3;
                time_intv.it_value.tv_nsec = 0;
                timerfd_settime(tfd,0,&time_intv,NULL);
                ev.data.fd = tfd;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,tfd,&ev);
                tfd2cfd[tfd] = cfd;
                climap[cfd] = cliinfo;
                continue;
            }
            //char req_content[1024];
            //int cur_req_content;
            // printf("cfd pollin\n");

            // 如是时间fd,说明超时了，服务器主动断开链接,客户端可能已经挂了
            // 清除缓存
            if(tfd2cfd.find(curfd)!=tfd2cfd.end())
            {
                ClientInfo *cliinfo = climap[tfd2cfd[curfd]];
                if(cliinfo==NULL) continue;
                shutdown(cliinfo->cfd,SHUT_RDWR);
                sprintf(logBuffer,"%s:%d timeout server closed",cliinfo->cliip,cliinfo->cliport); 
                free(climap[cliinfo->cfd]);
                auto iterclimap = climap.find(cliinfo->cfd);
                if(iterclimap!=climap.end()) climap.erase(iterclimap);
                auto itertfdmap = tfd2cfd.find(cliinfo->tfd);
                if(itertfdmap!=tfd2cfd.end()) tfd2cfd.erase(itertfdmap);
                epoll_ctl(epollfd,EPOLL_CTL_DEL,curfd,NULL);
                epoll_ctl(epollfd,EPOLL_CTL_DEL,cliinfo->tfd,NULL);
                YADILOGINFO(logBuffer); 
                continue;
            }
            if(climap.find(curfd)==climap.end()) continue;
            ClientInfo *cliinfo = climap[curfd];
            int reqlen = recv(curfd,cliinfo->req_content,1023,0);
            if(reqlen==-1||reqlen==0)
            {
                if(reqlen==-1&&(errno==EAGAIN||errno==EWOULDBLOCK))
                    continue;
                // method2: 对方关闭链接
                // shutdown(cliinfo->cfd,SHUT_RDWR);
                sprintf(logBuffer,"%s:%d client closed",cliinfo->cliip,cliinfo->cliport); 
                free(climap[cliinfo->cfd]);
                auto iterclimap = climap.find(cliinfo->cfd);
                if(iterclimap!=climap.end()) climap.erase(iterclimap);
                auto itertfdmap = tfd2cfd.find(cliinfo->tfd);
                if(itertfdmap!=tfd2cfd.end()) tfd2cfd.erase(itertfdmap);
                epoll_ctl(epollfd,EPOLL_CTL_DEL,curfd,NULL);
                epoll_ctl(epollfd,EPOLL_CTL_DEL,cliinfo->tfd,NULL);
                YADILOGINFO(logBuffer); 
                continue;
            }
            cliinfo->req_content[reqlen] = 0;
            
            char head[128];
            int ditmpi = 0;
            while(!(cliinfo->req_content[ditmpi]=='\r'&&cliinfo->req_content[ditmpi+1]=='\n'))
            {
                ditmpi++;
            }
            cliinfo->cur_req_content = ditmpi+2;
            strncpy(head,cliinfo->req_content,ditmpi);
            head[ditmpi] = 0;
            ditmpi = 0;
            while(isspace(head[ditmpi])) ++ditmpi;
            int ditmpj = ditmpi;
            while(!isspace(head[ditmpi])) ++ditmpi;
            strncpy(cliinfo->method,&head[ditmpj],ditmpi-ditmpj);
            cliinfo->method[ditmpi-ditmpj] = 0;
            sprintf(logBuffer,"%s:%d req method: %s",cliinfo->cliip,cliinfo->cliport,cliinfo->method); 
            YADILOGINFO(logBuffer); 
            if(strncmp(cliinfo->method,"GET",3)!=0)
            {

                sprintf(logBuffer,"%s:%d only GET method supported now! Got: %s\n",cliinfo->cliip,cliinfo->cliport,cliinfo->method);
                YADILOGINFO(logBuffer); 
                send(cliinfo->cfd,logBuffer,strlen(logBuffer),0);
                shutdown(cliinfo->cfd,SHUT_RDWR);
                continue;
            }
            while(isspace(head[ditmpi])) ++ditmpi;
            ditmpj =ditmpi;
            while(!isspace(head[ditmpi])) ++ditmpi;
            strncpy(cliinfo->filepath,&head[ditmpj],ditmpi-ditmpj);
            cliinfo->filepath[ditmpi-ditmpj] = 0;
            sprintf(logBuffer,"%s:%d req filepath: %s",cliinfo->cliip,cliinfo->cliport,cliinfo->filepath); 
            YADILOGINFO(logBuffer);
            sprintf(cliinfo->absoluteFilePath,"%s%s",rootdir,cliinfo->filepath);
            unsigned char *fileBuffer = (unsigned char *)malloc(sizeof(unsigned char)*1024*100);
            FILE *direqfd = fopen(cliinfo->absoluteFilePath,"rb");
            char outputhead[1024];
            char suffix[16];
            if(direqfd==NULL)
            {
                sprintf(logBuffer,"%s:%d 404 no such file.",cliinfo->cliip,cliinfo->cliport); 
                YADILOGINFO(logBuffer);
                sprintf(outputhead,"HTTP/1.1 404 not found\r\nServer:dihttpd\r\nContent-Type:text/plain\r\n\r\n");
                send(cliinfo->cfd,outputhead,strlen(outputhead),0);
                send(cliinfo->cfd,logBuffer,strlen(logBuffer),0);
                shutdown(cliinfo->cfd,SHUT_RDWR);
                continue;
            }
            size_t fileBufferlen = fread(fileBuffer,1,1024*100,direqfd);
            // printf("file buffer len: %d\n",fileBufferlen);
            char * pos = strrchr(cliinfo->filepath,'.');
            snprintf(suffix,15,"%s",&cliinfo->filepath[pos-cliinfo->filepath+1]);
            // printf("req type: %s\n",suffix);
            if(strncmp(suffix,"jpg",3)==0)
                sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:image/jpeg\r\n\r\n");
            else if(strncmp(suffix,"html",4)==0)
                sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
            else
                sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:text/plain\r\n\r\n");
            send(cliinfo->cfd,outputhead,strlen(outputhead),0);
            send(cliinfo->cfd,fileBuffer,fileBufferlen,0);
            
            free(fileBuffer);
            fclose(direqfd);
            shutdown(cliinfo->cfd,SHUT_RDWR);
        }
    }
    return true;
}