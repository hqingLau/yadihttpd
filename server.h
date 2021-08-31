/**
 ** by hqinglau 
 */

#ifndef YADI_SERVER_H
#define YADI_SERVER_H
#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "log.h"
#include <map>
using std::map;

namespace yadi
{
struct ClientInfo
{
    // cli info
    int fileBufferSent;
    int fileBufferlen;
    int cfd;
    FILE *fp;
    char req_content[1024];
    int cur_req_content;
    char cliip[64];
    int cliport;
    char method[16];
    char suffix[16];
    bool md2html;
    ssize_t fileBytesSent;
    int fileSize;
    char filepath[128];
    char absoluteFilePath[256];
    char httpversion[16];
    int tfd;
    unsigned char fileBuffer[1024*64];
};
class Server
{
private:
    // 权限管理
    uid_t euid;
    uid_t servuid;
    char ip[64];
    int port;
    char logPrefix[64];
    char logBuffer[256];
    char rootdir[64];
    int servSockfd;
    map<int,ClientInfo *> climap; // cfd->cline struct
    map<int,int> tfd2cfd; //tfd->cfd 定时用，超过时间断开链接
    int epollfd;
    int epollEvNum;
    epoll_event *srvEvents;
private:
    void handAccept();
    void handCliTimeout(int tfd);
    void cliCleaner(ClientInfo *cliinfo);
    void handSend(ClientInfo *cliinfo);
    void goDealWithPost(int cfd,char *path);
    int digetline(int cfd,char *buffer,int n);
public:
    Server(char *tip,int tport,char *webroot,char *ldir):port(tport){
        euid = geteuid();
        servuid = getuid();
        seteuid(servuid);
        epollfd = epoll_create(64);
        epollEvNum = 1024;
        srvEvents = (epoll_event *)malloc(sizeof(epoll_event)*epollEvNum);
        strncpy(ip,tip,63);
        // 放上我最喜欢的小猪猪作为前缀
        sprintf(logPrefix,"%s/yadilog",ldir);
        LOG::getInstance()->setPrefix(logPrefix);
        //strncpy(logPrefix,tlogPrefix,63);
        strcpy(rootdir,webroot);

        servSockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (servSockfd == -1)
        {
            sprintf(logBuffer, "%s: %s", "socket", strerror(errno));
            YADILOGERROR(logBuffer);
            handle_error("socket");
        }

        sockaddr_in sa;
        bzero(&sa, sizeof(sockaddr));
        sa.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &sa.sin_addr.s_addr);
        //sa.sin_addr.s_addr = inet_addr(ip); //INADDR_ANY;
        sa.sin_port = htons(port);
        if (bind(servSockfd, (sockaddr *)&sa, sizeof(sa)) == -1)
        {
            sprintf(logBuffer, "%s: %s", "bind", strerror(errno));
            YADILOGERROR(logBuffer);
            handle_error("bind");
        }

        if (listen(servSockfd, 10) == -1)
        {
            sprintf(logBuffer, "%s: %s", "listen", strerror(errno));
            YADILOGERROR(logBuffer);
            handle_error("listen");
        }

        sprintf(logBuffer, "%s:%d waiting for request...", ip, port);
        puts(logBuffer);
        YADILOGINFO(logBuffer);
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = servSockfd;
        setNonblock(servSockfd);
        epoll_ctl(epollfd, EPOLL_CTL_ADD, servSockfd, &ev);
    }
    ~Server()
    {
        free(srvEvents);
    }
    void run();
};
}

#endif //YADI_SERVER_H