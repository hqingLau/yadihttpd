#include "log.h"
#include "commonfunc.h"
#include <string.h>
#include <iostream>
#include <time.h>

yadi::LOG::LOG()
{
    // 暂时写死，要更改应该有个配置文件或者环境变量
    // todo
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
}

// 2018-05-22 15:35:53.850 TRACE
void yadi::LOG::log(LOGLEVEL level, char *msg,const char *file,const int line,const char *function)
{
    timeval tv;
    gettimeofday(&tv,0);
    tm *ditm = localtime(&tv.tv_sec);
    char ditimenow[64];
    strftime(ditimenow,sizeof(ditimenow),"%Y-%m-%d %H:%M:%S",ditm);
    snprintf(buffer,sizeof(buffer),"%s %s %s(%d)-<%s> %s\n",ditimenow,loglevelStr[level],file,line,function,msg);
    fputs(buffer,fp);
    fflush(fp);
}