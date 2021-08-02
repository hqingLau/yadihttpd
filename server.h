#ifndef YADI_SERVER_H
#define YADI_SERVER_H
#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include <map>
using std::map;

namespace yadi
{
struct ClientInfo
{
    // cli info
    int cfd;
    FILE *fp;
    char req_content[1024];
    int cur_req_content;
    char cliip[64];
    int cliport;
    char method[16];
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

public:
    Server(char *tip,int tport,char *tlogPrefix="log"):port(tport){
        epollfd = epoll_create(64);
        epollEvNum = 1024;
        srvEvents = (epoll_event *)malloc(sizeof(epoll_event)*epollEvNum);
        strncpy(ip,tip,63);
        strncpy(logPrefix,tlogPrefix,63);
        strcpy(rootdir,"/home/pi/www");
    }
    ~Server()
    {
        free(srvEvents);
    }
    bool run();
};
}

#endif //YADI_SERVER_H