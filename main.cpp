#include "server.h"
#include "log.h"

int main()
{
    YADILOGINFO("start main function");
    yadi::Server server("localhost",10240);
    server.run();
    
    return 0;
}