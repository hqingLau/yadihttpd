/**
 ** by hqinglau 
 */

#ifndef YADI_COMMONFUNC_H
#define YADI_COMMONFUNC_H
#include <unistd.h>
#include <fcntl.h>

#define handle_error(msg) \
    do {perror(msg);exit(EXIT_FAILURE);} while (0)

enum LOGLEVEL {
    LOGLEVEL_OFF,LOGLEVEL_FATAL,LOGLEVEL_ERROR,LOGLEVEL_WARN,LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,LOGLEVEL_TRACE,LOGLEVEL_ALL
};

int setNonblock(int fd);

#endif //YADI_COMMONFUNC_H