#include "log.h"
#include "commonfunc.h"
#include <string.h>
#include <iostream>
#include <time.h>

pthread_mutex_t logFileMtx = PTHREAD_MUTEX_INITIALIZER;

void *curQueue2file(void *parg)
{
    yadi::recordArg arg = *(yadi::recordArg *)parg;
    queue<yadi::logMsg *> *logQ = arg.logQ;
    // printf("len: %d\n",*(int *)arg.length);

    for(;;)
    {
        // 队列前面不需要加锁
        pthread_mutex_lock(&logFileMtx);
        if(logQ->empty()) 
        {
            pthread_mutex_unlock(&logFileMtx);
            sleep(1);
            continue;
        }
        yadi::logMsg *headMsg = logQ->front();
        
        fwrite(headMsg->msg,headMsg->length,1,*arg.fpp);
        free(headMsg->msg);
        free(headMsg);
        headMsg = nullptr;
        logQ->pop();
        pthread_mutex_unlock(&logFileMtx);
        fflush(*arg.fpp);
    }
}

yadi::LOG::LOG()
{
    // 暂时写死，要更改应该有个配置文件或者环境变量
    // todo
    lastRecord = time(NULL);
    recordInterval = 3; //3s记录到文件一次
    stackSize = 1024;
    mtx = PTHREAD_MUTEX_INITIALIZER;
    maxMsgNum = 10000;

    strncpy(prefix,"yadilog",63);
    timeval tv;
    gettimeofday(&tv,0);
    tm *ditm = localtime(&tv.tv_sec);
    char suffix[64];
    strftime(suffix,sizeof(suffix),"%Y%m%d%H%M%S",ditm);
    snprintf(filename,sizeof(filename),"%s_%s.log",prefix,suffix);
    // printf("%s\n",filename);
    fp = fopen(filename,"a");
    if(!fp) handle_error("log file open");
    arg.fpp = &fp;
    arg.logQ = &logQ;
    // curStack2file((void *)&arg);
    pthread_create(&pid,NULL,curQueue2file,(void *)&arg);
    pthread_detach(pid);
}

// log不再直接打印，而是放到一个64k的数组里（作为缓存区）
// 满了之后再打印（IO较慢）
void yadi::LOG::log(LOGLEVEL level, char *msg,const char *file,const int line,const char *function)
{
    timeval tv;
    gettimeofday(&tv,0);
    tm *ditm = localtime(&tv.tv_sec);
    char ditimenow[64];
    char buffer[1024];
    strftime(ditimenow,sizeof(ditimenow),"%Y-%m-%d %H:%M:%S",ditm);
    int sizeBuffer = snprintf(buffer,sizeof(buffer),"%s %s %s(%d)-<%s> %s\n",ditimenow,loglevelStr[level],file,line,function,msg);
    insertStack(buffer,sizeBuffer);
}



void yadi::LOG::insertStack(char *buffer,int sizeBuffer)
{
    char *msg = (char *)malloc(sizeof(char)*sizeBuffer);
    strncpy(msg,buffer,sizeBuffer);
    logMsg *lm = (logMsg *)malloc(sizeof(logMsg));
    lm->length = sizeBuffer;
    lm->msg = msg;
    pthread_mutex_lock(&logFileMtx);
    logQ.push(lm);
    pthread_mutex_unlock(&logFileMtx);
}
