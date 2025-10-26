#include "signal_handler.h"

int signal_caught = false;
struct sigaction new_action;
int socket_fd;
char * to_write;
char ip4[INET_ADDRSTRLEN]; 



void signal_handler(const int signal_no)
{
    signal_caught = true;
    syslog(LOG_ALERT, "Caught signal, exiting");
    syslog(LOG_ALERT, "Closed connection from %s",ip4);
    int remove_ret = remove(TEMP_FILE_PATH);
    if (remove_ret != ZERO)
    {
        syslog(LOG_ERR, "failed to remove %s",TEMP_FILE_PATH);
    }
    free(to_write);
    close(socket_fd);
    exit(0);
}


bool setup_signal_handler()
{
    bool success = true;
    new_action.sa_handler=signal_handler;
    if ( sigaction(SIGTERM, &new_action, NULL) != 0)
    {
        syslog(LOG_ERR, "error setting sigaction SIGTERM\n");
        success = false;
    }
    if ( sigaction(SIGINT, &new_action, NULL) != 0)
    {
        syslog(LOG_ERR, "error setting sigaction SIGINT\n");
        success = false;
    }

    if (success)
    {
        syslog(LOG_INFO, "signal setup successful\n");
    }
    return success;
}
