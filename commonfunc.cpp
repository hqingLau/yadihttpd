/**
 ** by hqinglau 
 */

#include "commonfunc.h"

int setNonblock(int fd)
{
    int flags = fcntl(fd,F_GETFL,0);
    flags|=O_NONBLOCK;
    fcntl(fd,F_SETFL,flags);
    return flags;
}
