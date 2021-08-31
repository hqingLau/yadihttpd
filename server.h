/**
 ** by hqinglau 
 */

#ifndef YADI_SERVER_H
#define YADI_SERVER_H
#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include "log.h"
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
    }
    ~Server()
    {
        free(srvEvents);
    }
    bool run();
};
}

#endif //YADI_SERVER_H