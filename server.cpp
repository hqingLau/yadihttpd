#include "server.h"
#include "commonfunc.h"
#include "log.h"
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>


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
    setNonblock(servSockfd);
    epoll_ctl(epollfd,EPOLL_CTL_ADD,servSockfd,&ev);

    int readyFdNum;
    int curfd;
    for(;;)
    {
        readyFdNum = epoll_wait(epollfd,srvEvents,epollEvNum,-1);
        for(int i=0;i<readyFdNum;i++)
        {
            curfd = srvEvents[i].data.fd;
            if(srvEvents[i].events & EPOLLIN)
            {
                if(curfd==servSockfd)
                {
                    handAccept();
                    continue;
                }


                // 如是时间fd,说明超时了，服务器主动断开链接,客户端可能已经挂了
                // 清除缓存
                if(tfd2cfd.find(curfd)!=tfd2cfd.end())
                {
                    handCliTimeout(curfd);
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
                    cliCleaner(cliinfo);
                    continue;
                }
                cliinfo->req_content[reqlen] = 0;
                
                char head[128];
                int ditmpi = 0;
                while(!(cliinfo->req_content[ditmpi]=='\r'&&cliinfo->req_content[ditmpi+1]=='\n'))
                {
                    ditmpi++;
		    // 不符合的url
		    if(ditmpi==125)
			break;
		}  
		if(ditmpi==125)
		{
			sprintf(logBuffer,"%s:%d url too long.",cliinfo->cliip,cliinfo->cliport,cliinfo->method);
			YADILOGINFO(logBuffer); 
			send(cliinfo->cfd,logBuffer,strlen(logBuffer),MSG_NOSIGNAL);
			shutdown(cliinfo->cfd,SHUT_RDWR);
			continue;
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
		    sprintf(logBuffer,"%s","yadihttpd error: url too long.");
                    send(cliinfo->cfd,logBuffer,strlen(logBuffer),MSG_NOSIGNAL);
                    shutdown(cliinfo->cfd,SHUT_RDWR);
                    continue;
                }
                while(isspace(head[ditmpi])) ++ditmpi;
                ditmpj =ditmpi;
                while(!isspace(head[ditmpi])) ++ditmpi;
                strncpy(cliinfo->filepath,&head[ditmpj],ditmpi-ditmpj);
                cliinfo->filepath[ditmpi-ditmpj] = 0;

                if(cliinfo->filepath[ditmpi-ditmpj-1]=='/')
                {
                    sprintf(cliinfo->filepath,"%s%s",cliinfo->filepath,"index.html");
                }
                sprintf(cliinfo->absoluteFilePath,"%s%s",rootdir,cliinfo->filepath);
                
                sprintf(logBuffer,"%s:%d req filepath: %s",cliinfo->cliip,cliinfo->cliport,cliinfo->filepath); 
                YADILOGINFO(logBuffer);

                FILE *direqfd = fopen(cliinfo->absoluteFilePath,"rb");
                char outputhead[1024*4];
                
                if(direqfd==NULL)
                {
                    sprintf(logBuffer,"%s:%d 404 no such file.",cliinfo->cliip,cliinfo->cliport); 
                    YADILOGINFO(logBuffer);
                    sprintf(outputhead,"HTTP/1.1 404 not found\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
                    send(cliinfo->cfd,outputhead,strlen(outputhead),MSG_NOSIGNAL);

			char fakehtmlPath[64];
			sprintf(fakehtmlPath,"%s%s",rootdir,"/md/fakeblog1.html");
			FILE *fakehtml1 = fopen(fakehtmlPath,"rb");
			size_t headlen = fread(outputhead,1,1024*4,fakehtml1);
			fclose(fakehtml1);
			write(cliinfo->cfd,outputhead,headlen);
			headlen = sprintf(outputhead,"<img src=\"https://gitee.com/hqinglau/img/raw/master/img/20210810112624.png\" alt=\"image-20210810112623229\" />");

			write(cliinfo->cfd,outputhead,headlen);
			sprintf(fakehtmlPath,"%s%s",rootdir,"/md/fakeblog2.html");
			FILE *fakehtml2 = fopen(fakehtmlPath,"rb");
			headlen = fread(outputhead,1,1024*4,fakehtml2);
			fclose(fakehtml2);
			write(cliinfo->cfd,outputhead,headlen);
                    shutdown(cliinfo->cfd,SHUT_RDWR);
                    continue;
                }
                char * pos = strrchr(cliinfo->filepath,'.');
                snprintf(cliinfo->suffix,15,"%s",&cliinfo->filepath[pos-cliinfo->filepath+1]);
                cliinfo->fp = direqfd;
                // 读取文件大小
                struct stat statbuf;
                stat(cliinfo->absoluteFilePath,&statbuf);
                cliinfo->fileSize = statbuf.st_size;
                
                
                // printf("file buffer len: %d\n",fileBufferlen);

                // printf("req type: %s\n",suffix);
                if(strncmp(cliinfo->suffix,"jpg",3)==0)
                    sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:image/jpeg\r\n\r\n");
                if(strncmp(cliinfo->suffix,"png",3)==0)
                    sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:image/png\r\n\r\n");

                if(strncmp(cliinfo->suffix,"ico",3)==0)
                    sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:image/x-icon\r\n\r\n");
                else if(strncmp(cliinfo->suffix,"html",4)==0)
                {
                    cliinfo->md2html = true;
                    sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
                }
                else if(strncmp(cliinfo->suffix,"js",2)==0)
                {
                    sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:application/javascript\r\n\r\n");
                }
                else if(strncmp(cliinfo->suffix,"css",3)==0)
                {
                    sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:text/css\r\n\r\n");
                }
                else
                    sprintf(outputhead,"HTTP/1.1 200 OK\r\nServer:dihttpd\r\nContent-Type:text/plain\r\n\r\n");
                write(cliinfo->cfd,outputhead,strlen(outputhead));

                // 如果是html，要发送个头
                if(cliinfo->md2html)
                {
                    char fakehtmlPath[64];
                    sprintf(fakehtmlPath,"%s%s",rootdir,"/md/fakeblog1.html");
                    FILE *fakehtml1 = fopen(fakehtmlPath,"rb");
                    size_t headlen = fread(outputhead,1,1024*4,fakehtml1);
                    fclose(fakehtml1);
                    write(cliinfo->cfd,outputhead,headlen);
                }
                handSend(cliinfo);

                // printf("filebytessent: %d, filesize: %d\n",cliinfo->fileBytesSent,cliinfo->fileSize);
                if(cliinfo->fileBytesSent==cliinfo->fileSize)
                {
                    if(cliinfo->md2html)
                    {
                        char fakehtmlPath[64];
                        sprintf(fakehtmlPath,"%s%s",rootdir,"/md/fakeblog2.html");
                        FILE *fakehtml2 = fopen(fakehtmlPath,"rb");
                        size_t headlen = fread(outputhead,1,1024*4,fakehtml2);
                        write(cliinfo->cfd,outputhead,headlen);
                        fclose(fakehtml2);
                    }
                    fclose(cliinfo->fp);
                    cliinfo->fp = nullptr;
                    shutdown(cliinfo->cfd,SHUT_RDWR);
                }
                else
                {
                    epoll_event diev;
                    diev.data.fd = cliinfo->cfd;
                    diev.events = EPOLLIN|EPOLLOUT;
                    epoll_ctl(epollfd,EPOLL_CTL_MOD,cliinfo->cfd,&diev);
                }
            }
            if(srvEvents[i].events & EPOLLOUT)
            {
                // 到这里的都是第一次没读完，接着读的
                ClientInfo *cliinfo = climap[curfd];
                if(cliinfo==NULL) continue;
                handSend(cliinfo);

                // printf("%s:%d sent: %d,size: %d\n",cliinfo->cliip,cliinfo->cliport,cliinfo->fileBytesSent,cliinfo->fileSize);
                if(cliinfo->fileBytesSent==cliinfo->fileSize)
                {
                    if(cliinfo->md2html)
                    {
                        char outputhead[1024];
                        char fakehtmlPath[64];
                        sprintf(fakehtmlPath,"%s%s",rootdir,"/md/fakeblog2.html");
                        FILE *fakehtml2 = fopen(fakehtmlPath,"rb");
                        size_t headlen = fread(outputhead,1,1024,fakehtml2);
                        write(cliinfo->cfd,outputhead,headlen);
                        fclose(fakehtml2);
                    }
                    epoll_event diev;
                    diev.data.fd = cliinfo->cfd;
                    diev.events = EPOLLIN;
                    epoll_ctl(epollfd,EPOLL_CTL_MOD,cliinfo->cfd,&diev);
                    fclose(cliinfo->fp);
                    cliinfo->fp = nullptr;
                    shutdown(cliinfo->cfd,SHUT_RDWR);
                }
            }
        }
    }
    return true;
}

void yadi::Server::handAccept()
{
    sockaddr_in sacli;
    socklen_t saclilen = sizeof(sacli);
    int cfd;
    cfd = accept(servSockfd,(sockaddr *)&sacli,&saclilen);
    if(cfd == -1) 
    {
        if(errno==EAGAIN||errno==EWOULDBLOCK) return;
        sprintf(logBuffer,"%s: %s","accept",strerror(errno)); 
        YADILOGERROR(logBuffer); 
        return;
    }
    ClientInfo *cliinfo = (ClientInfo *)malloc(sizeof(ClientInfo));
    inet_ntop(AF_INET,&sacli.sin_addr,cliinfo->cliip,sizeof(cliinfo->cliip));
    cliinfo->cliport = ntohs(sacli.sin_port);
    cliinfo->cfd = cfd;
    cliinfo->fileBytesSent = 0;
    cliinfo->md2html = false;
    sprintf(logBuffer,"%s:%d connected",cliinfo->cliip,cliinfo->cliport); 
    YADILOGINFO(logBuffer); 
    printf("%s\n",logBuffer);
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = cfd;
    setNonblock(cfd);
    epoll_ctl(epollfd,EPOLL_CTL_ADD,cfd,&ev);
    int tfd = timerfd_create(CLOCK_MONOTONIC,0);
    itimerspec time_intv;
    time_intv.it_value.tv_sec = 30;
    time_intv.it_value.tv_nsec = 0;
    timerfd_settime(tfd,0,&time_intv,NULL);
    ev.data.fd = tfd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,tfd,&ev);
    tfd2cfd[tfd] = cfd;
    cliinfo->tfd = tfd;
    cliinfo->fp = nullptr;
    climap[cfd] = cliinfo;
}


void yadi::Server::handCliTimeout(int curfd)
{
    ClientInfo *cliinfo = climap[tfd2cfd[curfd]];
    
    if(cliinfo==NULL) return;
    if(cliinfo->fp) fclose(cliinfo->fp);
    int tfd = cliinfo->tfd;
    int cfd = cliinfo->cfd;
    shutdown(cfd,SHUT_RDWR);
    sprintf(logBuffer,"%s:%d timeout server closed",cliinfo->cliip,cliinfo->cliport); 
    free(climap[cfd]);
    auto iterclimap = climap.find(cfd);
    if(iterclimap!=climap.end()) climap.erase(iterclimap);
    auto itertfdmap = tfd2cfd.find(tfd);
    if(itertfdmap!=tfd2cfd.end()) tfd2cfd.erase(itertfdmap);
    epoll_ctl(epollfd,EPOLL_CTL_DEL,curfd,NULL);
    epoll_ctl(epollfd,EPOLL_CTL_DEL,tfd,NULL);
    close(cfd);
    close(tfd);
    YADILOGINFO(logBuffer); 
}

void yadi::Server::cliCleaner(ClientInfo *cliinfo)
{
    sprintf(logBuffer,"%s:%d client closed",cliinfo->cliip,cliinfo->cliport); 
    int tfd = cliinfo->tfd;
    int cfd = cliinfo->cfd;
    if(cliinfo->fp) fclose(cliinfo->fp);
    auto iterclimap = climap.find(cliinfo->cfd);
    if(iterclimap!=climap.end()) 
    {
        free(climap[cliinfo->cfd]);
        climap.erase(iterclimap);
    }
    auto itertfdmap = tfd2cfd.find(tfd);
    if(itertfdmap!=tfd2cfd.end()) tfd2cfd.erase(itertfdmap);
    epoll_ctl(epollfd,EPOLL_CTL_DEL,cfd,NULL);
    epoll_ctl(epollfd,EPOLL_CTL_DEL,tfd,NULL);
    close(cfd);
    close(tfd);

    YADILOGINFO(logBuffer); 
}

void yadi::Server::handSend(ClientInfo *cliinfo)
{
    // 一次发送64kB
    size_t fileBufferlen =  fread(cliinfo->fileBuffer,1,1024*64,cliinfo->fp);
    size_t writensum = 0;
    size_t writen;
    while((writen = write(cliinfo->cfd,cliinfo->fileBuffer+writensum,fileBufferlen))!=-1)
    {
        writensum += writen;
        cliinfo->fileBytesSent += writen;
        // 正常发送了，再读
        if(writensum==fileBufferlen)
        {
            writensum = 0;
            fileBufferlen =  fread(cliinfo->fileBuffer,1,1024*64,cliinfo->fp);
            if(fileBufferlen==0) break;
        } 
    }
    if(writen==-1)
        fseek(cliinfo->fp,-(fileBufferlen-writensum),SEEK_CUR);  
}
