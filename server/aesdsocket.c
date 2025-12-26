
#include "signal_handler.h"
#include "socket_core.h"
#include <pthread.h>
#include <sched.h>
#include <sys/syslog.h>
extern pthread_mutex_t file_mutex;
extern pthread_mutex_t timer_mutex;
void* printing_time()
{

    pthread_cond_t cv;
    pthread_cond_init(&cv, NULL);
    pthread_mutex_init(&timer_mutex, NULL);
    pthread_mutex_init(&file_mutex,NULL);
    struct timespec time_to_wait;
    char date[100];
    while(1)
    {
        get_time(date);
        clock_gettime(CLOCK_REALTIME, &time_to_wait);
        time_to_wait.tv_sec = time_to_wait.tv_sec + 10;
        pthread_cond_timedwait(&cv, &timer_mutex, &time_to_wait);
        pthread_mutex_lock(&file_mutex);
        int fd = open(TEMP_FILE_PATH, O_SYNC| O_RDWR  |O_CREAT | O_APPEND, S_IWUSR |S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
        write(fd, date, strlen(date));
        fsync(fd);
        close(fd);        
        pthread_mutex_unlock(&file_mutex);
    }
}

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
            syslog(LOG_INFO,"starting aesd socket server . . .\n");
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, printing_time, NULL);
            socket_listen("");
    }
    exit(0);
}
