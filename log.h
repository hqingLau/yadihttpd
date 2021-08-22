/**
 ** by hqinglau 
 */

// 注：命名diXXX中，di无实际意义
#ifndef YADI_LOG_H
#define YADI_LOG_H

#include "commonfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <queue>
using std::queue;

/**
常见的日志格式中对于每一条日志应含有的信息包括日期、时间、日志级别、代码位置、日志内容、
错误码等信息。下面是一个工作中的日志文件的一部分内容：
2018-05-22 15:35:53.850 TRACE TDWZLog [0x00001b10] <36> <TDWZProtocol::Init>,TDWZPro
2018-05-22 15:35:53.850 TRACE TDWZLog [0x00001b10] <89> <TDWZProtocol::Init>,End 
 * 
*/
static const char *loglevelStr[8] = {"OFF","FATAL","ERROR","WARN","INFO","DEBUG","TRACE","ALL"};
void *curQueue2file(void *parg);

namespace yadi
{

struct logMsg
{
    char *msg;
    int length;
};

struct recordArg
{
    queue<logMsg *> *logQ;
    FILE **fpp;
    char **filename;
    char **prefix;
};

class LOG
{
private:
    char *prefix;
    char *filename;
    int msgNum; // log num
    int maxMsgNum; // change file if msgn>maxmsgn
    queue<logMsg *> logQ; 
    pthread_t pid; // 专门写curStack到磁盘的线程
    FILE *fp;
    time_t lastRecord;
    double recordInterval;
    recordArg arg;
    pthread_mutex_t mtx;
    int stackSize;


private:
    LOG();
    void insertStack(char *bufer,int sizeBuffer);
    void swapStackAndSignal();

public:
    static int logMsgCount; //5000条记录自动新建日志

public:
    static LOG* getInstance()
    {
        static LOG instance;
        return &instance;
    }

    // should be done only once
    void setPrefix(char *pf)
    {
        prefix = (char *)malloc(64);
        sprintf(prefix,pf);
         filename = (char *)malloc(64);
        strncpy(prefix,pf,63);
        timeval tv;
        gettimeofday(&tv,0);
        tm *ditm = localtime(&tv.tv_sec);
        char suffix[64];
        strftime(suffix,sizeof(suffix),"%Y%m%d%H%M%S",ditm);
        snprintf(filename,63,"%s_%s.log",prefix,suffix);
        // printf("%s\n",filename);
        fp = fopen(filename,"a");
        if(!fp) handle_error("log file open");
        arg.fpp = &fp;
        arg.logQ = &logQ;
        arg.filename = &filename;
        arg.prefix = &prefix;
        // curStack2file((void *)&arg);
        pthread_create(&pid,NULL,curQueue2file,(void *)&arg);
        pthread_detach(pid);
    }
    ~LOG() 
    {
        fclose(fp);
        free(filename);
        pthread_mutex_destroy(&mtx);
    }
    void log(LOGLEVEL level, char *msg,const char *file,const int line,const char *function);
};

}

// "OFF","FATAL","ERROR","WARN","INFO","DEBUG","TRACE","ALL"
#define YADILOGERROR(msg) yadi::LOG::getInstance()->log(LOGLEVEL_ERROR, msg,__FILE__,__LINE__,__FUNCTION__)
#define YADILOGFATAL(msg) yadi::LOG::getInstance()->log(LOGLEVEL_FATAL, msg,__FILE__,__LINE__,__FUNCTION__)
#define YADILOGWARN(msg) yadi::LOG::getInstance()->log(LOGLEVEL_WARN msg,__FILE__,__LINE__,__FUNCTION__)
#define YADILOGINFO(msg) yadi::LOG::getInstance()->log(LOGLEVEL_INFO, msg,__FILE__,__LINE__,__FUNCTION__)
#define YADILOGDEBUG(msg) yadi::LOG::getInstance()->log(LOGLEVEL_DEBUG, msg,__FILE__,__LINE__,__FUNCTION__)
#define YADILOGTRACE(msg) yadi::LOG::getInstance()->log(LOGLEVEL_TRACE, msg,__FILE__,__LINE__,__FUNCTION__)

#endif /* YADI_LOG_H */