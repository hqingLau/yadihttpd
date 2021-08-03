#include "log.h"
#include "commonfunc.h"
#include <string.h>
#include <iostream>
#include <time.h>
pthread_cond_t logCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t logCondMtx = PTHREAD_MUTEX_INITIALIZER;

void *curStack2file(void *parg)
{
    yadi::curStack2fileArg arg = *(yadi::curStack2fileArg *)parg;
    // printf("len: %d\n",*(int *)arg.length);
    for(;;)
    {
        pthread_mutex_lock(&logCondMtx);
        pthread_cond_wait(&logCond,&logCondMtx);
        // printf("%d\n",*(int *)arg.length);
        fwrite(*(arg.curStackPointer),*arg.length,1,*arg.fpp);
        fflush(*arg.fpp);
        pthread_mutex_unlock(&logCondMtx);
    }
}

yadi::LOG::LOG()
{
    // 暂时写死，要更改应该有个配置文件或者环境变量
    // todo
    lastRecord = time(NULL);
    recordInterval = 3; //3s记录到文件一次
    stackSize = 1024;
    curStack = (char *)malloc(sizeof(char)*stackSize);
    backStack = (char *)malloc(sizeof(char)*stackSize);

    mtx = PTHREAD_MUTEX_INITIALIZER;
    maxMsgNum = 10000;
    curStackIndex = (int *)malloc(sizeof(int));
    backStackIndex = (int *)malloc(sizeof(int));
    *curStackIndex = *backStackIndex = 0;
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
    
    arg.curStackPointer =  &backStack;
    arg.length = backStackIndex;

    arg.fpp = &fp;
    // curStack2file((void *)&arg);
    pthread_create(&pid,NULL,curStack2file,(void *)&arg);
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


void yadi::LOG::swapStackAndSignal()
{
    pthread_mutex_lock(&mtx);
    char *tmp = curStack;
    curStack = backStack;
    backStack = tmp;

    *backStackIndex = *curStackIndex;
    *curStackIndex = 0;
    pthread_cond_signal(&logCond);
    pthread_mutex_unlock(&mtx);
}

void yadi::LOG::insertStack(char *buffer,int sizeBuffer)
{
    // 单独搞个计时器好像没必要，会造成延迟，但是影响不大
    if((difftime(time(NULL),lastRecord)>recordInterval)||(*curStackIndex+sizeBuffer>stackSize-1))
    {
        // write buffer
        // 另开一个线程，条件量一直等待
        swapStackAndSignal();
        lastRecord = time(NULL);
    }
    strncpy(curStack+*curStackIndex,buffer,sizeBuffer);
    *curStackIndex+=sizeBuffer;
}
