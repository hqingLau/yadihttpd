// 注：命名diXXX中，di无实际意义
#include "commonfunc.h"
#include <stdio.h>
#include <sys/time.h>


/**
常见的日志格式中对于每一条日志应含有的信息包括日期、时间、日志级别、代码位置、日志内容、
错误码等信息。下面是一个工作中的日志文件的一部分内容：

2018-05-22 15:35:53.850 TRACE TDWZLog [0x00001b10] <36> <TDWZProtocol::Init>,TDWZPro
2018-05-22 15:35:53.850 TRACE TDWZLog [0x00001b10] <89> <TDWZProtocol::Init>,End 
2018-05-22 15:35:53.853 TRACE TDWZLog [0x00001b10] <142>    <TDWZProtocol::Connect>,
2018-05-22 15:35:53.853 TRACE TDWZLog [0x00002f10] <149>    <GetAlarmEventPro>,Enter
2018-05-22 15:39:36.382 WARN TrackLog [0x000029fc] - [ internal WARN htrace_server_

 * 
*/
static const char *loglevelStr[8] = {"OFF","FATAL","ERROR","WARN","INFO","DEBUG","TRACE","ALL"};


namespace yadi
{
class LOG
{
private:
    char prefix[64];
    char filename[64];
    char buffer[1024];
    int msgNum; // log num
    int maxMsgNum; // change file if msgn>maxmsgn
    FILE *fp;

private:
    LOG();
    

public:
    static LOG* getInstance()
    {
        static LOG instance;
        return &instance;
    }
    ~LOG() {fclose(fp);}
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