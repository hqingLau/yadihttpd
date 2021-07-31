#ifndef YADI_SERVER_H
#define YADI_SERVER_H
#include <iostream>
#include <string.h>


namespace yadi
{
class Server
{
private:
    char ip[64];
    char cliip[64];
    int port;
    int cliport;
    char logPrefix[64];
    char logBuffer[256];
    char rootdir[64];
    int servSockfd;
    char method[16];
    char filepath[128];
    char absoluteFilePath[256];
    char httpversion[16];
private:
    

public:
    Server(char *tip,int tport,char *tlogPrefix="log"):port(tport){
        strncpy(ip,tip,63);
        strncpy(logPrefix,tlogPrefix,63);
        strcpy(rootdir,"/home/pi/www");
    }
    bool run();
};
}

#endif //YADI_SERVER_H