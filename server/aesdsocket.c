
#include "signal_handler.h"


void* socket_listen(void * arg);
int main(int argc, char *argv[])
{
    if (setup_signal_handler() == FAILURE)
    {
        return -1;
    }
    int pid = 0;
    if (argc == TWO)
    {
        syslog(LOG_INFO, "the passed arguement is %s",argv[1]);
        if (strcmp(argv[1], "-d") == ZERO)
        {
            syslog(LOG_INFO, "launching in daemon mode. will fork the process");
            pid = fork();
            syslog(LOG_INFO, "PID is %i", pid);
        }
    }
    if (pid == ZERO)
    {
            printf("starting aesd socket server . . .\n");
            socket_listen("");
    }

    exit(0);
}
