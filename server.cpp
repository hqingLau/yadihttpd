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
#include <sys/stat.h>
#include <errno.h>

void garbageRecv(int curfd)
{
    int tmplen = 1;
    char buf[1025];
    while(tmplen>0){
        tmplen = recv(curfd, buf, 1024, 0);
    }
}

void yadi::Server::run()
{
    int readyFdNum;
    int curfd;
    for (;;)
    {
        readyFdNum = epoll_wait(epollfd, srvEvents, epollEvNum, -1);
        for (int i = 0; i < readyFdNum; i++)
        {
            curfd = srvEvents[i].data.fd;
            if (srvEvents[i].events & EPOLLIN)
            {
                //printf("curfd: %d\n",curfd);
                if (curfd == servSockfd)
                {
                    handAccept();
                    continue;
                }

                // 如是时间fd,说明超时了，服务器主动断开链接,客户端可能已经挂了
                // 清除缓存
                if (tfd2cfd.find(curfd) != tfd2cfd.end())
                {
                    handCliTimeout(curfd);
                    continue;
                }
                if (climap.find(curfd) == climap.end())
                    continue;
                ClientInfo *cliinfo = climap[curfd];
                if(!cliinfo) continue;
                char head[128];
                char outputhead[1024 * 16];
                int ditmpi = 0;

                int reqlen = 0;
                int tmplen;
                char lastc = 0;
                char curc = 0;
                int di_stat = 0;
                while(1) 
                {
                    tmplen = recv(curfd, &cliinfo->req_content[reqlen], 1, 0);
                    if (tmplen == -1 || tmplen == 0)
                    {
                        if (tmplen == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                            continue;
                        // method2: 对方关闭链接
                        // shutdown(cliinfo->cfd,SHUT_RDWR);
                        cliCleaner(cliinfo);
                        di_stat = 1;
                        break;
                    }
                    lastc = curc;
                    curc = cliinfo->req_content[reqlen];
                    reqlen++;
                    if ((lastc == '\r' && curc == '\n')||reqlen==125)
                    {
                        break;
                    }
                }
                if(di_stat==1) continue;
                ditmpi = reqlen;
                cliinfo->req_content[reqlen] = 0;
                if (ditmpi == 125)
                {
                    sprintf(logBuffer, "%s:%d url too long.", cliinfo->cliip, cliinfo->cliport, cliinfo->method);
                    YADILOGINFO(logBuffer);
                    sprintf(outputhead, "HTTP/1.1 404 not found\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
                    send(cliinfo->cfd, outputhead, strlen(outputhead), MSG_NOSIGNAL);

                    char fakehtmlPath[64];
                    sprintf(fakehtmlPath, "%s%s", rootdir, "/404.html");
                    FILE *fakehtml1 = fopen(fakehtmlPath, "rb");
                    size_t headlen = fread(outputhead, 1, 1024 * 16, fakehtml1);
                    fclose(fakehtml1);
                    send(cliinfo->cfd, outputhead, headlen,MSG_NOSIGNAL);


                    shutdown(cliinfo->cfd, SHUT_RDWR);
                    continue;
                }

                strncpy(head, cliinfo->req_content, ditmpi);
                head[ditmpi] = 0;
                ditmpi = 0;
                while (isspace(head[ditmpi]))
                    ++ditmpi;
                int ditmpj = ditmpi;
                while (!isspace(head[ditmpi]))
                    ++ditmpi;
                strncpy(cliinfo->method, &head[ditmpj], ditmpi - ditmpj);
                cliinfo->method[ditmpi - ditmpj] = 0;
                sprintf(logBuffer, "%s:%d req method: %s", cliinfo->cliip, cliinfo->cliport, cliinfo->method);
                YADILOGINFO(logBuffer);
                while (isspace(head[ditmpi]))
                    ++ditmpi;
                ditmpj = ditmpi;
                while (!isspace(head[ditmpi]))
                    ++ditmpi;
                strncpy(cliinfo->filepath, &head[ditmpj], ditmpi - ditmpj);
                cliinfo->filepath[ditmpi - ditmpj] = 0;
                if (strncmp(cliinfo->method, "GET", 3) != 0)
                {

                    sprintf(logBuffer, "%s:%d only GET and Post method supported now! Got: %s\n", cliinfo->cliip, cliinfo->cliport, cliinfo->method);
                    YADILOGINFO(logBuffer);
                    if (strncmp(cliinfo->method, "POST", 4) == 0)
                    {
                        char path[256];
                        strncpy(path,cliinfo->filepath,255);
                        goDealWithPost(cliinfo->cfd,path);
                        continue;
                    }
                    garbageRecv(cliinfo->cfd);
                    sprintf(outputhead, "HTTP/1.1 404 not found\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
                    send(cliinfo->cfd, outputhead, strlen(outputhead), MSG_NOSIGNAL);

                    char fakehtmlPath[64];
                    sprintf(fakehtmlPath, "%s%s", rootdir, "/404.html");
                    FILE *fakehtml1 = fopen(fakehtmlPath, "rb");
                    size_t headlen = fread(outputhead, 1, 1024 * 16, fakehtml1);
                    fclose(fakehtml1);
                    send(cliinfo->cfd, outputhead, headlen,MSG_NOSIGNAL);
                    shutdown(cliinfo->cfd, SHUT_RDWR);
                    continue;
                }
                garbageRecv(cliinfo->cfd);
                int tempi = 0;
                for (tempi = 0; tempi < ditmpi - ditmpj; tempi++)
                {
                    if (cliinfo->filepath[tempi] == '?')
                    {
                        cliinfo->filepath[tempi] = 0;
                        break;
                    }
                }
                if (cliinfo->filepath[tempi - 1] == '/')
                {
                    sprintf(cliinfo->filepath, "%s%s", cliinfo->filepath, "index.html");
                }
                sprintf(cliinfo->absoluteFilePath, "%s%s", rootdir, cliinfo->filepath);

                sprintf(logBuffer, "%s:%d req filepath: %s", cliinfo->cliip, cliinfo->cliport, cliinfo->filepath);
                YADILOGINFO(logBuffer);
                FILE *direqfd = NULL;
                int is404 = 0;
                struct stat statbuf;
                int fileStat = stat(cliinfo->absoluteFilePath, &statbuf);
                if (fileStat != 0)
                {
                    is404 = 1;
                }
                else if (S_ISDIR(statbuf.st_mode) == 1)
                {
                    is404 = 1;
                }
                // 增加文件权限管理
                else if (statbuf.st_uid != euid)
                {
                    is404 = 1;
                }
                direqfd = fopen(cliinfo->absoluteFilePath, "rb");

                if (direqfd == NULL || is404 == 1)
                {
                    sprintf(logBuffer, "%s:%d 404 no such file.", cliinfo->cliip, cliinfo->cliport);
                    YADILOGINFO(logBuffer);
                    sprintf(outputhead, "HTTP/1.1 404 not found\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
                    send(cliinfo->cfd, outputhead, strlen(outputhead), MSG_NOSIGNAL);

                    char fakehtmlPath[64];
                    sprintf(fakehtmlPath, "%s%s", rootdir, "/404.html");
                    FILE *fakehtml1 = fopen(fakehtmlPath, "rb");
                    size_t headlen = fread(outputhead, 1, 1024 * 16, fakehtml1);
                    fclose(fakehtml1);
                    send(cliinfo->cfd, outputhead, headlen,MSG_NOSIGNAL);
                    shutdown(cliinfo->cfd, SHUT_RDWR);
                    continue;
                }
                char *pos = strrchr(cliinfo->filepath, '.');
                snprintf(cliinfo->suffix, 15, "%s", &cliinfo->filepath[pos - cliinfo->filepath + 1]);
                cliinfo->fp = direqfd;
                // 读取文件大小
                cliinfo->fileSize = statbuf.st_size;

                // printf("file buffer len: %d\n",fileBufferlen);

                // printf("req type: %s\n",suffix);
                if (strncmp(cliinfo->suffix, "jpg", 3) == 0)
                    sprintf(outputhead, "HTTP/1.1 200 OK\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:image/jpeg\r\n\r\n");
                if (strncmp(cliinfo->suffix, "png", 3) == 0)
                    sprintf(outputhead, "HTTP/1.1 200 OK\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:image/png\r\n\r\n");

                if (strncmp(cliinfo->suffix, "ico", 3) == 0)
                    sprintf(outputhead, "HTTP/1.1 200 OK\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:image/x-icon\r\n\r\n");
                else if (strncmp(cliinfo->suffix, "html", 4) == 0)
                {
                    cliinfo->md2html = true;
                    sprintf(outputhead, "HTTP/1.1 200 OK\r\nServer:dihttpd\r\nConnection:close\r\nContent-Type:text/html\r\n\r\n");
                }
                else if (strncmp(cliinfo->suffix, "js", 2) == 0)
                {
                    sprintf(outputhead, "HTTP/1.1 200 OK\r\nServer:dihttpd\r\nConnection:close\r\nContent-Type:application/javascript\r\n\r\n");
                }
                else if (strncmp(cliinfo->suffix, "css", 3) == 0)
                {
                    sprintf(outputhead, "HTTP/1.1 200 OK\r\nServer:dihttpd\r\nConnection:close\r\nContent-Type:text/css\r\n\r\n");
                }
                else
                    sprintf(outputhead, "HTTP/1.1 200 OK\r\nServer:dihttpd\r\nConnection:close\r\nContent-Type:text/plain\r\n\r\n");
                send(cliinfo->cfd, outputhead, strlen(outputhead),MSG_NOSIGNAL);

                handSend(cliinfo);

                // printf("filebytessent: %d, filesize: %d\n",cliinfo->fileBytesSent,cliinfo->fileSize);
                if (cliinfo->fileBytesSent == cliinfo->fileSize)
                {
                    fclose(cliinfo->fp);
                    cliinfo->fp = nullptr;
                    shutdown(cliinfo->cfd, SHUT_RDWR);
                }
                else
                {
                    epoll_event diev;
                    diev.data.fd = cliinfo->cfd;
                    diev.events =  EPOLLOUT;
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, cliinfo->cfd, &diev);
                }
            }
            if (srvEvents[i].events & EPOLLOUT)
            {
                // 到这里的都是第一次没读完，接着读的
                ClientInfo *cliinfo = climap[curfd];
                if (cliinfo == NULL)
                    continue;
                handSend(cliinfo);

                // printf("%s:%d sent: %d,size: %d\n",cliinfo->cliip,cliinfo->cliport,cliinfo->fileBytesSent,cliinfo->fileSize);
                if (cliinfo->fileBytesSent == cliinfo->fileSize)
                {
                    epoll_event diev;
                    diev.data.fd = cliinfo->cfd;
                    diev.events = EPOLLIN;
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, cliinfo->cfd, &diev);
                    if(cliinfo->fp)
                        fclose(cliinfo->fp);
                    cliinfo->fp = nullptr;
                    garbageRecv(cliinfo->cfd);
                    shutdown(cliinfo->cfd, SHUT_RDWR);
                }
            }
        }
    }
}

void yadi::Server::handAccept()
{
    sockaddr_in sacli;
    socklen_t saclilen = sizeof(sacli);
    int cfd;
    cfd = accept(servSockfd, (sockaddr *)&sacli, &saclilen);
    if (cfd == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        sprintf(logBuffer, "%s: %s", "accept", strerror(errno));
        YADILOGERROR(logBuffer);
        return;
    }
    ClientInfo *cliinfo = (ClientInfo *)malloc(sizeof(ClientInfo));
    inet_ntop(AF_INET, &sacli.sin_addr, cliinfo->cliip, sizeof(cliinfo->cliip));
    cliinfo->cliport = ntohs(sacli.sin_port);
    cliinfo->cfd = cfd;
    cliinfo->fileBytesSent = 0;
    cliinfo->fileBufferSent = cliinfo->fileBufferlen = 0;
    cliinfo->md2html = false;
    sprintf(logBuffer, "%s:%d connected", cliinfo->cliip, cliinfo->cliport);
    YADILOGINFO(logBuffer);
    printf("%s\n", logBuffer);
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = cfd;
    setNonblock(cfd);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev);
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    itimerspec time_intv;
    time_intv.it_value.tv_sec = 60;
    time_intv.it_value.tv_nsec = 0;
    time_intv.it_interval.tv_sec = 0;
    time_intv.it_interval.tv_nsec = 0;
    if (timerfd_settime(tfd, 0, &time_intv, NULL) == -1)
        perror("timerfd settime\n");
    ev.data.fd = tfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, tfd, &ev);
    tfd2cfd[tfd] = cfd;
    cliinfo->tfd = tfd;
    //	printf("tfd: %d\n",tfd);
    cliinfo->fp = nullptr;
    climap[cfd] = cliinfo;
}

void yadi::Server::handCliTimeout(int curfd)
{
    //printf("in handCliTimeout\n");
    ClientInfo *cliinfo = climap[tfd2cfd[curfd]];
    if(!cliinfo) return;
    if (cliinfo->fp){
        fclose(cliinfo->fp);
        cliinfo->fp=nullptr;
    }
    int tfd = cliinfo->tfd;
    int cfd = cliinfo->cfd;
    shutdown(cfd, SHUT_RDWR);
    sprintf(logBuffer, "%s:%d timeout server closed", cliinfo->cliip, cliinfo->cliport);
    auto iterclimap = climap.find(cfd);
    if (iterclimap != climap.end())
    {
        free(climap[cliinfo->cfd]);
        climap.erase(iterclimap);
    }
    auto itertfdmap = tfd2cfd.find(tfd);
    if (itertfdmap != tfd2cfd.end())
        tfd2cfd.erase(itertfdmap);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, curfd, NULL);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, tfd, NULL);
    close(cfd);
    close(tfd);
    cliinfo = nullptr;
    YADILOGINFO(logBuffer);
}

void yadi::Server::cliCleaner(ClientInfo *cliinfo)
{
    if(!cliinfo) return;
    sprintf(logBuffer, "%s:%d client closed", cliinfo->cliip, cliinfo->cliport);
    int tfd = cliinfo->tfd;
    int cfd = cliinfo->cfd;
    if (cliinfo->fp){
        fclose(cliinfo->fp);
        cliinfo->fp=nullptr;
    }
    auto iterclimap = climap.find(cliinfo->cfd);
    if (iterclimap != climap.end())
    {
        free(climap[cliinfo->cfd]);
        climap.erase(iterclimap);
    }
    auto itertfdmap = tfd2cfd.find(tfd);
    if (itertfdmap != tfd2cfd.end())
        tfd2cfd.erase(itertfdmap);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, cfd, NULL);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, tfd, NULL);
    close(cfd);
    close(tfd);
    cliinfo = nullptr;
    YADILOGINFO(logBuffer);
}

void yadi::Server::handSend(ClientInfo *cliinfo)
{
    // 一次发送16kB
    if(!cliinfo->fp) return;
    if(cliinfo->fileBufferSent==cliinfo->fileBufferlen)
    {
        cliinfo->fileBufferlen = fread(cliinfo->fileBuffer, 1, 1024 * 32, cliinfo->fp);
        cliinfo->fileBufferSent=0;
        if(cliinfo->fileBufferlen<=0) return;
    }
    
    int sentN = send(cliinfo->cfd, cliinfo->fileBuffer+cliinfo->fileBufferSent, cliinfo->fileBufferlen-cliinfo->fileBufferSent,MSG_NOSIGNAL);
    if(sentN<=0) return;
    cliinfo->fileBufferSent += sentN;
    cliinfo->fileBytesSent += sentN;
}

int yadi::Server::digetline(int cfd,char *buffer,int n)
{
    int reqlen = 0;
    int tmplen;
    char lastc = 0;
    char curc = 0;
    while(1) 
    {
        tmplen = recv(cfd, &buffer[reqlen], 1, 0);
        if (tmplen == -1 || tmplen == 0)
        {
            return -1;
        }
        lastc = curc;
        curc = buffer[reqlen];
        reqlen++;  
        if(reqlen==n)
        {
            garbageRecv(cfd);
            sprintf(buffer, "HTTP/1.1 404 not found\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
            send(cfd, buffer, strlen(buffer), MSG_NOSIGNAL);
            shutdown(cfd,  SHUT_RDWR );
            return -1;
        }
        if ((lastc == '\r' && curc == '\n'))
        {
            buffer[reqlen-2] = 0;
            return reqlen-2;
        }
    }
}

void shutDownPost(int cfd,char *buffer)
{
    garbageRecv(cfd);
    sprintf(buffer, "HTTP/1.1 404 not found\r\nConnection:close\r\nServer:dihttpd\r\nContent-Type:text/html\r\n\r\n");
    send(cfd, buffer, strlen(buffer), MSG_NOSIGNAL);
    shutdown(cfd, SHUT_RDWR);
}

void yadi::Server::goDealWithPost(int cfd,char *path)
{
    char buffer[1024 * 16];
    if(strcmp(path,"/blogUpload")!=0)
    {
        shutDownPost(cfd,buffer);
        return;
    }

    char contentype[]="Content-Type: multipart/form-data; boundary=";
    char boundary[64] = {0};
    boundary[0] = boundary[1] = '-';
    while(1)
    {
        if(digetline(cfd,buffer,1024)==-1) {
            shutDownPost(cfd,buffer);
            return;
        }
        //到了post数据区域
        if(strlen(buffer)==0)
            break;
        if(strncmp(buffer,contentype,strlen(contentype))==0)
        {
            strcpy(boundary+2,buffer+strlen(contentype));
            printf("%s\n",boundary);
        }
    }
    if(strlen(boundary)==0)
    {
        shutDownPost(cfd,buffer);
        return;
    }
    char passwd[128];
    char lname[128];
    char filename[128];
    char name[64];
    if((digetline(cfd,buffer,1024)==-1)||(strcmp(buffer,boundary)!=0))
    {
        shutDownPost(cfd,buffer);
        return;
    }
    // 下方是post数据

    while(1)
    {
        if(digetline(cfd,buffer,1024)==-1)
            break;
        char *nameptr = strstr(buffer,"name=\"");
        if(!nameptr) continue;
        int i = 0;
        nameptr = nameptr+6;
        while(nameptr[i]!='\"')
        {
            if(i==60) break;
            name[i]=nameptr[i];
            i++;
        }
        name[i]=0;

        nameptr = strstr(buffer,"filename=\"");
        i = 0;
        if(nameptr)
        {
            nameptr = nameptr+10;
            while(nameptr[i]!='\"')
            {
                if(i==127) break;
                filename[i]=nameptr[i];
                i++;
            }
            filename[i]=0;
        }
        

        if(strcmp(name,"lname")==0)
        {
            while(strlen(buffer)>0) digetline(cfd,buffer,1024);
            digetline(cfd,buffer,1024);
            strncpy(lname,buffer,127);
        }

        else if(strcmp(name,"passwd")==0)
        {
            while(strlen(buffer)>0) digetline(cfd,buffer,1024);
            digetline(cfd,buffer,1024);
            strncpy(passwd,buffer,127);
            if(strcmp(passwd,"leiyadi")!=0)
            {
                shutDownPost(cfd,buffer);
                if(strlen(filename)!=0) unlink(filename);
                return;
            }
        }
        else if(strcmp(name,"file")==0)
        {
            while(strlen(buffer)>0) digetline(cfd,buffer,1024);
            
            if(strlen(filename)==0)
            {
                shutDownPost(cfd,buffer);
                return;
            }
            int curc = 0;
            int lastc = 0;
            int curIndex = 0;
            char fullPath[128];
            sprintf(fullPath, "%s/md/%s", rootdir,filename);
            FILE *fp = fopen(fullPath,"wb");

            int tmplen;
            int endFlag = 0;
            while(1) 
            {
                tmplen = recv(cfd, &buffer[curIndex], 1, 0);
                if (tmplen == -1 || tmplen == 0)
                {
                    shutDownPost(cfd,buffer);
                    return;
                }
                curc = buffer[curIndex];
                curIndex++;  
                while (curc == '\n')
                {
                    //\n之前先写入
                    fwrite(buffer,1,curIndex-2,fp);
                    fflush(fp);
                    buffer[0] = curc;
                    curIndex=1;
                    for(i=0;i<strlen(boundary);i++)
                    {
                        tmplen = recv(cfd, &buffer[curIndex], 1, 0);
                        curc = buffer[curIndex];
                        if (tmplen == -1 || tmplen == 0)
                        {
                            shutDownPost(cfd,buffer);
                            return;
                        }
                        if(buffer[curIndex]!=boundary[i])
                        {
                            fwrite(buffer,1,curIndex,fp);
                            fflush(fp);
                            buffer[0] = curc;
                            curIndex=1;
                            break;
                        }
                        else
                        {
                            curIndex++;
                        }
                    }
                    if(i==strlen(boundary))
                    {
                        endFlag = 1;
                        break;
                    }

                }
                if(endFlag==1)
                {
                    endFlag = 0;
                    fclose(fp);
                    break;
                }
                if(curIndex==1024*16)
                {
                    fwrite(buffer,1,curIndex-1,fp);
                    fflush(fp);
                    curIndex = 0;
                }
                
            }
        }
    }
    if(strcmp(passwd,"leiyadi")!=0)
    {
        unlink(filename);
        return;
    }
    if(strlen(lname)>0) {
        for(int i=0;i<strlen(filename);i++)
        {
            if(filename[i]=='.')
            {
                filename[i] = 0;
                break;
            }
        }
        sprintf(buffer,"%s %s\n",filename,lname);
        char fullPath[128];
        sprintf(fullPath, "%s/md/%s", rootdir,"filename2name.txt");
        FILE *addItemfp = fopen(fullPath,"a");
        fwrite(buffer,1,strlen(buffer),addItemfp);
        fclose(addItemfp);
        sprintf(fullPath, "%s/md/%s", rootdir,"md2html.sh");
        if(fork()==0)
        {
            char *arg[] = {"./md2html.sh",NULL};
            execv(fullPath,arg);
        }
        shutdown(cfd, SHUT_RDWR);
    } 
}